/**
 * @file   dnsUtilTest.c
 * @author night1700 <night1700@night1700-15ZB970-GP50ML>
 * @date   Tue Mar  9 11:15:29 2021
 * 
 * @brief  
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "dnsUtil.h"
 
int main(int argc , char **argv)
{
	if(argc != 3) {
		fprintf(stderr, "usage: dnsQuery [hostname] [dns_svr_ip]\n");
		exit(0);
	}
    char ip[48];
    unsigned char hostname[100];
	strcpy((char*)hostname, argv[1]);
	fprintf(stderr, "hostname=[%s]\n", hostname);

	strcpy(dns_servers[0] , argv[2]);
	fprintf(stderr, "dns server=[%s]\n", dns_servers[0]);

    getHostFirstIpByName(ip, (char*)hostname, dns_servers[0]);
    printf("ip=%s hostname=%s dns_servers=%s\n", ip, hostname, dns_servers[0]);
    return 0;
}
