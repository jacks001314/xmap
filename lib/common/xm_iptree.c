/*
 * =====================================================================================
 *
 *       Filename:  xm_iptree.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/12/2019 10:01:27 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_iptree.h"
#include <stdint.h>
#include <stdio.h>
#include "xm_xalloc.h"

#define IS_LEAF(node) ((node)->l == NULL)

static xm_node_t *_create_leaf(uint32_t value)
{
	xm_node_t *node = xmalloc(sizeof(xm_node_t));
	node->l = NULL;
	node->r = NULL;
	node->value = value;
	return node;
}

static void _destroy_subtree(xm_node_t *node)
{
	if (node == NULL)
		return;
	_destroy_subtree(node->l);
	_destroy_subtree(node->r);
	free(node);
}

static void _convert_to_leaf(xm_node_t *node)
{
    if(node == NULL||!IS_LEAF(node))
        return;

	_destroy_subtree(node->l);
	_destroy_subtree(node->r);
	node->l = NULL;
	node->r = NULL;
}

static void _set_recurse(xm_node_t *node, uint32_t prefix, int len, uint32_t value)
{

    if(node == NULL ||len<0||len>256)
        return;

	if (len == 0) {
		// We're at the end of the prefix; make this a leaf and set the
		// value.
		if (!IS_LEAF(node)) {
			_convert_to_leaf(node);
		}
		node->value = value;
		return;
	}

	if (IS_LEAF(node)) {
		// We're not at the end of the prefix, but we hit a leaf.
		if (node->value == value) {
			// A larger prefix has the same value, so we're done.
			return;
		}
		// The larger prefix has a different value, so we need to
		// convert it into an internal node and continue processing on
		// one of the leaves.
		node->l = _create_leaf(node->value);
		node->r = _create_leaf(node->value);
	}

	// We're not at the end of the prefix, and we're at an internal
	// node.  Recurse on the left or right subtree.
	if (prefix & 0x80000000) {
		_set_recurse(node->r, prefix << 1, len - 1, value);
	} else {
		_set_recurse(node->l, prefix << 1, len - 1, value);
	}

	// At this point, we're an internal node, and the value is set
	// by one of our children or its descendent.  If both children are
	// leaves with the same value, we can discard them and become a left.
	if (IS_LEAF(node->r) && IS_LEAF(node->l) &&
	    node->r->value == node->l->value) {
		node->value = node->l->value;
		_convert_to_leaf(node);
	}
}

// Set the value for a given network prefix, overwriting any existing
// values on that prefix or subsets of it.
// (Note: prefix must be in host byte order.)
void xm_iptree_add(xm_iptree_t *iptree, uint32_t prefix, int len, uint32_t value)
{
	if(iptree == NULL)
        return;

	_set_recurse(iptree->root, prefix, len, value);
	iptree->painted = 0;
}

// Return the value pertaining to an address, according to the tree
// starting at given root.  (Note: address must be in host byte order.)
static int _lookup_ip(xm_node_t *root, uint32_t address)
{
	xm_node_t *node = root;
	uint32_t mask = 0x80000000;
	for (;;) {
		if (IS_LEAF(node)) {
			return node->value;
		}
		if (address & mask) {
			node = node->r;
		} else {
			node = node->l;
		}
		mask >>= 1;
	}
}

// Return the value pertaining to an address.
// (Note: address must be in host byte order.)
uint32_t xm_iptree_lookup_ip(xm_iptree_t *iptree, uint32_t address)
{
	return _lookup_ip(iptree->root, address);
}

// Return the nth painted IP address.
static int _lookup_index(xm_node_t *root, uint64_t n)
{
	xm_node_t *node = root;
	uint32_t ip = 0;
	uint32_t mask = 0x80000000;
	for (;;) {
		if (IS_LEAF(node)) {
			return ip | n;
		}
		if (n < node->l->count) {
			node = node->l;
		} else {
			n -= node->l->count;
			node = node->r;
			ip |= mask;
		}
		mask >>= 1;
	}
}

// For a given value, return the IP address with zero-based index n.
// (i.e., if there are three addresses with value 0xFF, looking up index 1
// will return the second one).
// Note that the tree must have been previously painted with this value.
uint32_t xm_iptree_lookup_index(xm_iptree_t *iptree, uint64_t index,
				 uint32_t value)
{
	if (!iptree->painted || iptree->paint_value != value) {
		xm_iptree_paint_value(iptree, value);
	}

	uint64_t radix_idx = index / (1 << (32 - RADIX_LENGTH));
	if (radix_idx < iptree->radix_len) {
		// Radix lookup
		uint32_t radix_offset =
		    index % (1 << (32 - RADIX_LENGTH)); // TODO: bitwise maths
		return iptree->radix[radix_idx] | radix_offset;
	}

	// Otherwise, do the "slow" lookup in tree.
	// Note that tree counts do NOT include things in the radix,
	// so we subtract these off here.
	index -= iptree->radix_len * (1 << (32 - RADIX_LENGTH));
	return _lookup_index(iptree->root, index);
}

// Implement count_ips by recursing on halves of the tree.  Size represents
// the number of addresses in a prefix at the current level of the tree.
// If paint is specified, each node will have its count set to the number of
// leaves under it set to value.
// If exclude_radix is specified, the number of addresses will exlcude prefixes
// that are a /RADIX_LENGTH or larger
static uint64_t _count_ips_recurse(xm_node_t *node, uint32_t value, uint64_t size,
				   int paint, int exclude_radix)
{
	uint64_t n;
	if (IS_LEAF(node)) {
		if (node->value == value) {
			n = size;
			// Exclude prefixes already included in the radix
			if (exclude_radix &&
			    size >= (1 << (32 - RADIX_LENGTH))) {
				n = 0;
			}
		} else {
			n = 0;
		}
	} else {
		n = _count_ips_recurse(node->l, value, size >> 1, paint,
				       exclude_radix) +
		    _count_ips_recurse(node->r, value, size >> 1, paint,
				       exclude_radix);
	}
	if (paint) {
		node->count = n;
	}
	return n;
}

// Return a node that determines the values for the addresses with
// the given prefix.  This is either the internal node that
// corresponds to the end of the prefix or a leaf node that
// encompasses the prefix. (Note: prefix must be in host byte order.)
static xm_node_t *_lookup_node(xm_node_t *root, uint32_t prefix, int len)
{
	xm_node_t *node = root;
	uint32_t mask = 0x80000000;
	int i;

	for (i = 0; i < len; i++) {
		if (IS_LEAF(node)) {
			return node;
		}
		if (prefix & mask) {
			node = node->r;
		} else {
			node = node->l;
		}
		mask >>= 1;
	}
	return node;
}

// For each node, precompute the count of leaves beneath it set to value.
// Note that the tree can be painted for only one value at a time.
void xm_iptree_paint_value(xm_iptree_t *iptree, uint32_t value)
{

	// Paint everything except what we will put in radix
	_count_ips_recurse(iptree->root, value, (uint64_t)1 << 32, 1, 1);

	// Fill in the radix array with a list of addresses
	uint32_t i;
	iptree->radix_len = 0;
	for (i = 0; i < (1 << RADIX_LENGTH); i++) {
		uint32_t prefix = i << (32 - RADIX_LENGTH);
		xm_node_t *node = _lookup_node(iptree->root, prefix, RADIX_LENGTH);
		if (IS_LEAF(node) && node->value == value) {
			// Add this prefix to the radix
			iptree->radix[iptree->radix_len++] = prefix;
		}
	}
	iptree->painted = 1;
	iptree->paint_value = value;
}

// Return the number of addresses that have a given value.
uint64_t xm_iptree_count_ips(xm_iptree_t *iptree, uint32_t value)
{
	if (iptree->painted && iptree->paint_value == value) {
		return iptree->root->count +
		       iptree->radix_len * (1 << (32 - RADIX_LENGTH));
	} else {
		return _count_ips_recurse(iptree->root, value, (uint64_t)1 << 32,
					  0, 0);
	}
}

// Initialize the tree.
// All addresses will initially have the given value.
xm_iptree_t *xm_iptree_create(uint32_t value)
{
	xm_iptree_t *iptree = (xm_iptree_t*)xmalloc(sizeof(xm_iptree_t));
	iptree->root = _create_leaf(value);
	iptree->radix = xcalloc(sizeof(uint32_t), 1 << RADIX_LENGTH);
	iptree->painted = 0;
	return iptree;
}

// Deinitialize and free the tree.
void xm_iptree_destroy(xm_iptree_t *iptree)
{
    if(iptree == NULL)
        return;

	_destroy_subtree(iptree->root);
	free(iptree->radix);
	free(iptree);
}

