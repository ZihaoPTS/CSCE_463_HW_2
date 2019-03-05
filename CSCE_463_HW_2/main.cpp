#include "stdafx.h"

void makeDNSquestion(char* buf, char* host) {
	//printf("host:%s\n", host);
	char* buf_ptr = buf;
	char* host_ptr = host;
	int counter = 0;
	while (true) {
		if (host_ptr[counter] == '.') {
			buf_ptr[0] = char(counter);
			buf_ptr += (counter+1);
			host_ptr += (counter+1); 
			counter = 0;
		}
		else if (host_ptr[counter] == '\0'){
			buf_ptr[0] = char(counter);
			buf_ptr[counter + 1] = 0;
			break;
		}
		else{
			buf_ptr[counter+1] = host_ptr[counter];
			counter++;
		}
	}

	//printf("String:%s\nI'm another line", buf);
	return;
}
void printQType(u_short qT) {
	u_short qTs[] = {DNS_A,DNS_NS,DNS_CNAME,DNS_PTR,
		DNS_HINFO,DNS_MX,DNS_AXFR,DNS_ANY};
	switch (qT)
	{
	default:
		break;
	case DNS_A:
		printf(" DNS_A ");
		break;
	case DNS_NS:
		printf(" DNS_NS ");
		break;
	case DNS_CNAME:
		printf(" DNS_CNAME ");
		break;
	case DNS_PTR:
		printf(" DNS_PTR ");
		break;
	case DNS_HINFO:
		printf(" DNS_HINFO ");
		break;
	case DNS_MX:
		printf(" DNS_MX ");
		break;
	case DNS_AXFR:
		printf(" DNS_AXFR ");
		break;
	case DNS_ANY:
		printf(" DNS_ANY ");
		break;
	}
	
}

u_char* jumpNoutput(u_char* answer) { // returning a pointer at next answer
	u_char* ans = answer;
	int curPos = 0;
	int pnj = 0; //position next to jump
	int off = 0;
	bool jumped = false;
	//print answer_name
	while(true){
		if (answer[curPos] >= 0xc0) {
			off = ((answer[curPos] & 0x3F) << 8) + answer[curPos + 1];
			curPos += off;
			pnj = curPos+2;
			jumped = true;
		}
		else if (answer[curPos] == 0) {
			if (jumped){
				curPos = pnj;
				jumped = false;
			}
			else
				break;
		}
		else {
			int count = answer[curPos];
			curPos += 1;
			for (int i = 0; i < count; i++) {
				printf("%c", answer[curPos]);
				curPos++;
			}
		}

	}
	
	//compute FixedRR, print Qtype
	ans += (curPos + 1);
	FixedRR* frr = (FixedRR*)ans;
	ans = (u_char*)(frr + 1);
	printQType(frr->qT);
	
	//print answer
	if(frr->qT == DNS_A) //print ip adderss
		for (int i = 0; i < frr->len; i++) {
			printf("%d.", ans[i]);
		}
	else { //print name
		curPos = 0;
		while (true) {
			if (ans[curPos] >= 0xc0) {
				off = ((ans[curPos] & 0x3F) << 8) + ans[curPos + 1];
				curPos += off;
				pnj = curPos + 2;
				jumped = true;
			}
			else if (ans[curPos] == 0) {
				if (jumped){
					curPos = pnj;
					jumped = false;
				}
				else
					break;
			}
			else {
				int count = ans[curPos];
				curPos += 1;
				for (int i = 0; i < count; i++) {
					printf("%c", ans[curPos]);
					curPos++;
				}
			}

		}
	}
	//print rest of Fixed RR
	
	
	//compute return address
	return ans;
}


void makeRDNSquestion(char* buf, char* host) {
	return;
}

int main(int argc, char** argv) {
	//argv[1] : host name/IP
	//argv[2] : DNS server IP

	WSADATA wsaData;

	//Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return 0;
	}

	if (argc != 3) {
		printf("Wrong Input method. Exiting Program");
		exit(0);
	}

	u_short QueryType;

	DWORD IP = inet_addr(argv[1]);
	if (IP == INADDR_NONE)
	{
		//Not a IP
		//Query Type A
		QueryType = DNS_A;
	}
	else
	{
		//Is a IP
		//Query Type PTR
		QueryType = DNS_PTR;
	}

	SOCKET SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// handle errors
	if (SendSocket == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		exit(0);
	}

	//if(DNS_PTR == DNS_PTR) then do following
	int pkt_size = strlen(argv[1]) + 2 + sizeof(FixedDNSheader) + sizeof(QueryHeader);
	char *buf = new char[pkt_size];

	FixedDNSheader *fdh = (FixedDNSheader *)buf;
	QueryHeader *qh = (QueryHeader*)(buf + pkt_size - sizeof(QueryHeader));
	
	// fixed field initialization
	fdh->ID = htons(1);
	fdh->flags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);
	fdh->nAdditional = 0;
	fdh->nAnswers = 0;
	fdh->nAuthority = 0;
	fdh->nQuestions = htons(1);
	qh->qType = htons(QueryType);
	qh->qClass = htons(DNS_INET);

	makeDNSquestion((char*)(fdh + 1), argv[1]);
	//else do reverse look up


	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(0);
	if (bind(SendSocket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		printf("bind() generated error %d\n", WSAGetLastError());
		if (closesocket(SendSocket) == SOCKET_ERROR) {
			printf("recvfrom() generated error %d\n", WSAGetLastError());
		}
		WSACleanup();
		exit(0);
	}

	struct sockaddr_in remote;
	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(argv[2]); // server
	remote.sin_port = htons(53);
	if (sendto(SendSocket, buf, pkt_size, 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR) {
		printf("sendto() generated error %d\n", WSAGetLastError());
		if (closesocket(SendSocket) == SOCKET_ERROR) {
			printf("recvfrom() generated error %d\n", WSAGetLastError());
		}
		WSACleanup();
		exit(0);
	}

	char* recv_buf = new char[MAX_DNS_LEN];
	struct sockaddr_in RecvAddr;
	int RecvAddrSize = sizeof(RecvAddr);
	int count = 0;
	while (count++ < MAX_ATTEMPTS)
	{
		timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		// get ready to receive
		fd_set fd;
		FD_ZERO(&fd); // clear the set
		FD_SET(SendSocket, &fd); // add your socket to the set
		int available = select(0, &fd, NULL, NULL, &timeout);
		//int available = 1;
		if (available > 0)
		{
			int size = -1;
			if ((size = recvfrom(SendSocket, recv_buf, MAX_DNS_LEN, 0, (SOCKADDR *)& RecvAddr, &RecvAddrSize)) == SOCKET_ERROR) {
				printf("recvfrom() generated error %d\n", WSAGetLastError());
				if (closesocket(SendSocket) == SOCKET_ERROR) {
					printf("recvfrom() generated error %d\n", WSAGetLastError());
				}
				WSACleanup();
				exit(0);
			}
			printf("size received: %d\n",size);
			// parse the response
			break;
		}
		else if (available == 0) {
			// report timeout
			printf("\tSelect timeout\n");
			break;
		}
		else {
			printf("\tSelect error: %d\n", WSAGetLastError());
			break;
		}
		printf("Max Attempt Reached. No select evoke successfully\n");
	}
	// some error checking here

	// call cleanup when done with everything and ready to exit program
	if (closesocket(SendSocket) == SOCKET_ERROR) {
		printf("recvfrom() generated error %d\n", WSAGetLastError());
	}
	WSACleanup();

	if (RecvAddr.sin_addr.s_addr != remote.sin_addr.s_addr || RecvAddr.sin_port != remote.sin_port){
		printf("Reply address did not match send address. Exit\n");
		if (closesocket(SendSocket) == SOCKET_ERROR) {
			printf("recvfrom() generated error %d\n", WSAGetLastError());
		}
		WSACleanup();
		exit(0);
	}

	FixedDNSheader *Recv_fdh = (FixedDNSheader*)recv_buf;

	printf("TXID 0x%X, flags 0x%X, questions %d, answers %d, authority %d, additional %d\n",
		ntohs(Recv_fdh->ID), ntohs(Recv_fdh->flags), ntohs(Recv_fdh->nQuestions),
		ntohs(Recv_fdh->nAnswers), ntohs(Recv_fdh->nAuthority),
		ntohs(Recv_fdh->nAdditional));

	// my question + Query + frr  + answer
	FixedRR *frr = (FixedRR*)(recv_buf + pkt_size);

	//printf("qC:%d,TTL:%d,len:%d\n", ntohs(frr->qC), ntohs(frr->TTL), ntohs(frr->len));

	u_char* answer = (u_char*)(frr + 1);

	if (int(answer) >= 0xC0)
		printf("gonna jump\n");
	else
		printf("not gonna jump");

	//printf("recv_buf: %s\n", recv_buf);

	//need to do
	//apply recursive serving
	//find respond (error)
	
	//

	return 0;
}