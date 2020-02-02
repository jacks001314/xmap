/*
 * =====================================================================================
 *
 *       Filename:  xm_probe.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/30/2019 01:24:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jacksha (shajf), csp001314@163.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include "xm_probe.h"

extern xm_probe_t probe_tcp_synscan;
extern xm_probe_t probe_tcp_synackscan;
extern xm_probe_t probe_icmp_echo;
extern xm_probe_t probe_icmp_echo_time;

static xm_probe_t *probes[] = {
    &probe_tcp_synscan,
    &probe_tcp_synackscan, 
    &probe_icmp_echo,
    &probe_icmp_echo_time
    // ADD YOUR MODULE HERE
};

xm_probe_t* xm_probe_get_by_name(const char *name)
{
    int i;
	int len = (int)(sizeof(probes) / sizeof(probes[0]));

    if(name == NULL ||strlen(name)==0)
        return NULL;

	for (i = 0; i < len; i++) {
		if (!strcmp(probes[i]->name, name)) {

			return probes[i];
		}
	}

	return NULL;
}

void xm_probes_dump(void)
{
    int i;
	int len = (int)(sizeof(probes) / sizeof(probes[0]));

	for (i = 0; i < len; i++) {
		printf("%s\n", probes[i]->name);
	}
}

