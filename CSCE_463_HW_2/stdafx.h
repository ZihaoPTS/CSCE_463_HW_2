#pragma once
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include "stdlib.h"
#include <cstring>
#include <string>
#include "mysocket.h"
#pragma comment(lib, "Ws2_32.lib")

/* DNS query types */
#define DNS_A 1			/* name -> IP */
#define DNS_NS 2		/* name server */ 
#define DNS_CNAME 5		/* canonical name */ 
#define DNS_PTR 12		/* IP -> name */
#define DNS_HINFO 13	/* host info/SOA */
#define DNS_MX 15		/* mail exchange */
#define DNS_AXFR 252	/* request for zone transfer */
#define DNS_ANY 255		/* all records */

/* query classes */
#define DNS_INET 1 

#define MAX_DNS_LEN 512

#define MAX_ATTEMPTS 3 

/* flags */
#define DNS_QUERY		(0 << 15) /* 0 = query; 1 = response */
#define DNS_RESPONSE	(1 << 15)

#define DNS_STDQUERY	(0 << 11) /* opcode - 4 bits */

#define DNS_AA			(1 << 10)	/* authoritative answer */
#define NS_TC			(1 << 9)	/* truncated */
#define DNS_RD			(1 << 8)	/* recursion desired*/
#define DNS_RA			(1 << 7)	/* recursion available*/