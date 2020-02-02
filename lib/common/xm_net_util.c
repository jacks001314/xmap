/*
 *
 *      Filename: xm_net_util.c
 *
 *        Author: shajf,csp001314@gmail.com
 *   Description: ---
 *        Create: 2018-05-17 11:40:55
 * Last Modified: 2018-07-11 14:01:34
 */

#include "xm_net_util.h"
#include <errno.h>
#include "xm_headers.h"
#include "xm_log.h"
#include <pcap.h>

static const char * const fast_strings[] = {
"0", "1", "2", "3", "4", "5", "6", "7",
"8", "9", "10", "11", "12", "13", "14", "15",
"16", "17", "18", "19", "20", "21", "22", "23",
"24", "25", "26", "27", "28", "29", "30", "31",
"32", "33", "34", "35", "36", "37", "38", "39",
"40", "41", "42", "43", "44", "45", "46", "47",
"48", "49", "50", "51", "52", "53", "54", "55",
"56", "57", "58", "59", "60", "61", "62", "63",
"64", "65", "66", "67", "68", "69", "70", "71",
"72", "73", "74", "75", "76", "77", "78", "79",
"80", "81", "82", "83", "84", "85", "86", "87",
"88", "89", "90", "91", "92", "93", "94", "95",
"96", "97", "98", "99", "100", "101", "102", "103",
"104", "105", "106", "107", "108", "109", "110", "111",
"112", "113", "114", "115", "116", "117", "118", "119",
"120", "121", "122", "123", "124", "125", "126", "127",
"128", "129", "130", "131", "132", "133", "134", "135",
"136", "137", "138", "139", "140", "141", "142", "143",
"144", "145", "146", "147", "148", "149", "150", "151",
"152", "153", "154", "155", "156", "157", "158", "159",
"160", "161", "162", "163", "164", "165", "166", "167",
"168", "169", "170", "171", "172", "173", "174", "175",
"176", "177", "178", "179", "180", "181", "182", "183",
"184", "185", "186", "187", "188", "189", "190", "191",
"192", "193", "194", "195", "196", "197", "198", "199",
"200", "201", "202", "203", "204", "205", "206", "207",
"208", "209", "210", "211", "212", "213", "214", "215",
"216", "217", "218", "219", "220", "221", "222", "223",
"224", "225", "226", "227", "228", "229", "230", "231",
"232", "233", "234", "235", "236", "237", "238", "239",
"240", "241", "242", "243", "244", "245", "246", "247",
"248", "249", "250", "251", "252", "253", "254", "255"
};

#define MAX_IP_STR_LEN 16
char*
xm_ip_to_str(char *buf, size_t buf_len,uint32_t ip)
{
    char const *p;
    char *b=buf;
	const uint8_t* ad = (const uint8_t*)&ip;

	if(buf_len < MAX_IP_STR_LEN || ip == 0){
        return "0.0.0.0";
    }
	
    p=fast_strings[*ad++];
    do {
        *b++=*p;
        p++;
    } while(*p);
    *b++='.';

    p=fast_strings[*ad++];
    do {
        *b++=*p;
        p++;
    } while(*p);
    *b++='.';

    p=fast_strings[*ad++];
    do {
        *b++=*p;
        p++;
    } while(*p);
    *b++='.';

    p=fast_strings[*ad];
    do {
        *b++=*p;
        p++;
    } while(*p);
    *b=0;

	return buf;
}

int read_nl_sock(int sock, char *buf, int buf_len)
{
	int msg_len = 0;
	char *pbuf = buf;
	do {
		int len = recv(sock, pbuf, buf_len - msg_len, 0);
		if (len <= 0) {
			xm_log(XM_LOG_ERR,"get-gw recv failed: %s", strerror(errno));
			return -1;
		}
		struct nlmsghdr *nlhdr = (struct nlmsghdr *)pbuf;
		if (NLMSG_OK(nlhdr, ((unsigned int)len)) == 0 ||
		    nlhdr->nlmsg_type == NLMSG_ERROR) {
			xm_log(XM_LOG_ERR,"get-gw recv failed: %s", strerror(errno));
			return -1;
		}
		if (nlhdr->nlmsg_type == NLMSG_DONE) {
			break;
		} else {
			msg_len += len;
			pbuf += len;
		}
		if ((nlhdr->nlmsg_flags & NLM_F_MULTI) == 0) {
			break;
		}
	} while (1);
	return msg_len;
}

int send_nl_req(uint16_t msg_type, uint32_t seq, void *payload,
		uint32_t payload_len)
{
	int sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (sock < 0) {
		xm_log(XM_LOG_ERR,"get-gw unable to get socket: %s",
			  strerror(errno));
		return -1;
	}

	if (NLMSG_SPACE(payload_len) < payload_len) {
		close(sock);
		// Integer overflow
		return -1;
	}
	struct nlmsghdr *nlmsg;
	nlmsg = malloc(NLMSG_SPACE(payload_len));

	memset(nlmsg, 0, NLMSG_SPACE(payload_len));
	memcpy(NLMSG_DATA(nlmsg), payload, payload_len);
	nlmsg->nlmsg_type = msg_type;
	nlmsg->nlmsg_len = NLMSG_LENGTH(payload_len);
	nlmsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	nlmsg->nlmsg_seq = seq;
	nlmsg->nlmsg_pid = getpid();

	if (send(sock, nlmsg, nlmsg->nlmsg_len, 0) < 0) {
		xm_log(XM_LOG_ERR,"get-gw failure sending: %s", strerror(errno));
		return -1;
	}
	free(nlmsg);
	return sock;
}

int xm_get_iface_ip(const char *iface, struct in_addr *ip)
{
	int sock;
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		xm_log(XM_LOG_ERR,"get-iface-ip failure opening socket: %s",
			  strerror(errno));
        return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
		close(sock);
		xm_log(XM_LOG_ERR,"get-iface-ip ioctl failure: %s", strerror(errno));
        return -1;
	}

	ip->s_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	close(sock);
	return 0;
}

int xm_get_iface_hw_addr(const char *iface, uint8_t *hw_mac)
{
	int s;
	struct ifreq buffer;

	// Load the hwaddr from a dummy socket
	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		xm_log(XM_LOG_ERR,"get_iface_hw_addr Unable to open socket: %s",
			  strerror(errno));

		return -1;
	}
	
    memset(&buffer, 0, sizeof(buffer));
	
    strncpy(buffer.ifr_name, iface, IFNAMSIZ);
	
    ioctl(s, SIOCGIFHWADDR, &buffer);
	
    close(s);
	
    memcpy(hw_mac, buffer.ifr_hwaddr.sa_data, 6);
	
    return 0;
}


char *xm_get_default_iface(void)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	char *iface = pcap_lookupdev(errbuf);
	if (iface == NULL) {
		xm_log(XM_LOG_ERR,"xmap could not detect default network interface "
				  "(e.g. eth0). Try running as root or setting"
				  " interface using -i flag.");
		return NULL;
	}

	return iface;
}

// gw and iface[IF_NAMESIZE] MUST be allocated
int _get_default_gw(struct in_addr *gw, char *iface)
{
	struct rtmsg req;
	unsigned int nl_len;
	char buf[8192];
	struct nlmsghdr *nlhdr;

	if (!gw || !iface) {
		return -1;
	}

	// Send RTM_GETROUTE request
	memset(&req, 0, sizeof(req));
	int sock = send_nl_req(RTM_GETROUTE, 0, &req, sizeof(req));

	// Read responses
	nl_len = read_nl_sock(sock, buf, sizeof(buf));
	if (nl_len <= 0) {
		return -1;
	}

	// Parse responses
	nlhdr = (struct nlmsghdr *)buf;
	while (NLMSG_OK(nlhdr, nl_len)) {
		struct rtattr *rt_attr;
		struct rtmsg *rt_msg;
		int rt_len;
		int has_gw = 0;

		rt_msg = (struct rtmsg *)NLMSG_DATA(nlhdr);

		if ((rt_msg->rtm_family != AF_INET) ||
		    (rt_msg->rtm_table != RT_TABLE_MAIN)) {
			return -1;
		}

		rt_attr = (struct rtattr *)RTM_RTA(rt_msg);
		rt_len = RTM_PAYLOAD(nlhdr);
		while (RTA_OK(rt_attr, rt_len)) {
			switch (rt_attr->rta_type) {
			case RTA_OIF:
				if_indextoname(*(int *)RTA_DATA(rt_attr),
					       iface);
				break;
			case RTA_GATEWAY:
				gw->s_addr = *(unsigned int *)RTA_DATA(rt_attr);
				has_gw = 1;
				break;
			}
			rt_attr = RTA_NEXT(rt_attr, rt_len);
		}

		if (has_gw) {
			return 0;
		}
		nlhdr = NLMSG_NEXT(nlhdr, nl_len);
	}
	return -1;
}

int xm_get_default_gw(struct in_addr *gw, char *iface)
{
	char _iface[IF_NAMESIZE];
	memset(_iface, 0, IF_NAMESIZE);

	_get_default_gw(gw, _iface);
	if (strcmp(iface, _iface) != 0) {
		xm_log(XM_LOG_ERR,
		    "get-gateway interface specified (%s) does not match "
		    "the interface of the default gateway (%s). You will need "
		    "to manually specify the MAC address of your gateway.",
		    iface, _iface);
        return -1;
	}
	return 0;
}

int xm_get_hw_addr(struct in_addr *gw_ip, char *iface, unsigned char *hw_mac)
{
	struct ndmsg req;
	memset(&req, 0, sizeof(struct ndmsg));

	if (!gw_ip || !hw_mac) {
		return -1;
	}
	// Send RTM_GETNEIGH request
	req.ndm_family = AF_INET;
	req.ndm_ifindex = if_nametoindex(iface);
	req.ndm_state = NUD_REACHABLE;
	req.ndm_type = NDA_LLADDR;

	int sock = send_nl_req(RTM_GETNEIGH, 1, &req, sizeof(req));
	// Read responses
	char *buf = malloc(GW_BUFFER_SIZE);
	int nl_len = read_nl_sock(sock, buf, GW_BUFFER_SIZE);
	if (nl_len <= 0) {
		free(buf);
		return -1;
	}
	// Parse responses
	struct nlmsghdr *nlhdr = (struct nlmsghdr *)buf;
	while (NLMSG_OK(nlhdr, nl_len)) {
		struct rtattr *rt_attr;
		struct rtmsg *rt_msg;
		int rt_len;
		unsigned char mac[6];
		struct in_addr dst_ip;
		int correct_ip = 0;

		rt_msg = (struct rtmsg *)NLMSG_DATA(nlhdr);
		if ((rt_msg->rtm_family != AF_INET)) {
			free(buf);
			return -1;
		}
		rt_attr = (struct rtattr *)RTM_RTA(rt_msg);
		rt_len = RTM_PAYLOAD(nlhdr);
		while (RTA_OK(rt_attr, rt_len)) {
			switch (rt_attr->rta_type) {
			case NDA_LLADDR:
				if (RTA_PAYLOAD(rt_attr) != IFHWADDRLEN) {
					// could be using a VPN
					xm_log(XM_LOG_ERR,
					    "get_gateway Unexpected hardware address length (%d)."
					    " If you are using a VPN, supply the --iplayer flag (and provide an"
					    " interface via -i)",
					    RTA_PAYLOAD(rt_attr));
                    return -1;
				}
				memcpy(mac, RTA_DATA(rt_attr), IFHWADDRLEN);
				break;
			case NDA_DST:
				if (RTA_PAYLOAD(rt_attr) != sizeof(dst_ip)) {
					// could be using a VPN
					xm_log(XM_LOG_ERR,
					    "get_gateway Unexpected IP address length (%d)."
					    " If you are using a VPN, supply the --iplayer flag"
					    " (and provide an interface via -i)",
					    RTA_PAYLOAD(rt_attr));
                    return -1;
				}
				memcpy(&dst_ip, RTA_DATA(rt_attr),
				       sizeof(dst_ip));
				if (memcmp(&dst_ip, gw_ip, sizeof(dst_ip)) ==
				    0) {
					correct_ip = 1;
				}
				break;
			}
			rt_attr = RTA_NEXT(rt_attr, rt_len);
		}
		if (correct_ip) {
			memcpy(hw_mac, mac, IFHWADDRLEN);
			free(buf);
			return 0;
		}
		nlhdr = NLMSG_NEXT(nlhdr, nl_len);
	}
	free(buf);
	return -1;
}

