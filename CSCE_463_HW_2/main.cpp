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
		printf(" A ");
		break;
	case DNS_NS:
		printf(" NS ");
		break;
	case DNS_CNAME:
		printf(" CNAME ");
		break;
	case DNS_PTR:
		printf(" PTR ");
		break;
	case DNS_HINFO:
		printf(" HINFO ");
		break;
	case DNS_MX:
		printf(" MX ");
		break;
	case DNS_AXFR:
		printf(" AXFR ");
		break;
	case DNS_ANY:
		printf(" ANY ");
		break;
	}
	
}

int printQuestion(u_char* recv_buf, int offset, bool print) { // returning a pointer at next answer
	int curPos = offset;
	int pnj = 0; //position next to jump
	int off = 0;
	bool jumped = false;
	bool first = false;
	//print answer_name
	while(true){
		if (recv_buf[curPos] >= 0xc0) {
			off = ((recv_buf[curPos] & 0x3F) << 8) + recv_buf[curPos + 1];
			if (!jumped)
				pnj = curPos + 2;
			curPos = off;
			jumped = true;
		}
		else if (recv_buf[curPos] == 0) {
			if (jumped){
				curPos = pnj;
				return curPos;
			}
			else {
				curPos += 1;
				return curPos;
			}
		}
		else {
			if (print&first)
				printf(".");
			else
				first = true;
			int count = recv_buf[curPos];
			curPos ++;
			for (int i = 0; i < count; i++) {
				if(print)
					printf("%c", recv_buf[curPos]);
				curPos++;
			}
		}

	}
	
	//compute FixedRR, print Qtype
	//if(nev_jumped)
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
	char* question;
	DWORD IP = inet_addr(argv[1]);
	if (IP == INADDR_NONE)
	{
		//Not a IP
		//Query Type A
		QueryType = DNS_A;
		question = argv[1];
	}
	else
	{
		//Is a IP
		//Query Type PTR
		QueryType = DNS_PTR;//inet_ntoa

		DWORD test = htonl(inet_addr(argv[1]));
		struct sockaddr_in addr;
		addr.sin_addr.S_un.S_addr = test;

		question = new char[strlen(argv[1]) + strlen(".in-addr.arpa")+1];
		memcpy(question, inet_ntoa(addr.sin_addr), strlen(inet_ntoa(addr.sin_addr)));
		int curPos = strlen(inet_ntoa(addr.sin_addr));
		memcpy(question + curPos, ".in-addr.arpa", strlen(".in-addr.arpa"));
		question[strlen(argv[1]) + strlen(".in-addr.arpa")] = '\0';

		printf("%s\n", question);
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
	

	printf("Lookup: %s\n",question);
	printf("Query : %s, type %d, TXID %X\n",question,QueryType,fdh->ID);
	printf("Server : %s\n",argv[2]);
	printf("********************************\n");

	char* recv_buf = new char[MAX_DNS_LEN];
	struct sockaddr_in RecvAddr;
	int RecvAddrSize = sizeof(RecvAddr);
	int count = 0;
	while (count++ < MAX_ATTEMPTS)
	{
		clock_t t1, t2;
		t1 = clock();
		printf("Attempt %d with %d bytes...",count,pkt_size);
		if (sendto(SendSocket, buf, pkt_size, 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR) {
			printf("sendto() generated error %d\n", WSAGetLastError());
			if (closesocket(SendSocket) == SOCKET_ERROR) {
				printf("recvfrom() generated error %d\n", WSAGetLastError());
			}
			WSACleanup();
			exit(0);
		}
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
			t2 = clock();
			float diff((float)t2 - (float)t1);
			float seconds = diff / CLOCKS_PER_SEC;
			printf("\tresponse in %.3f s with %d bytes\n", seconds,size);
			// parse the response
			break;
		}
		else if (available == 0) {
			// report timeout
			t2 = clock();
			float diff((float)t2 - (float)t1);
			float seconds = diff / CLOCKS_PER_SEC;
			printf("\ttimeout in %.3f s\n",seconds);

			break;
		}
		else {
			printf("\tSelect error: %d\n", WSAGetLastError());
			if (closesocket(SendSocket) == SOCKET_ERROR) {
				printf("recvfrom() generated error %d\n", WSAGetLastError());
			}
			WSACleanup();
			exit(0);
		}
		printf("\tMax Attempt Reached. No select evoke successfully. Exiting Program\n");
		if (closesocket(SendSocket) == SOCKET_ERROR) {
			printf("recvfrom() generated error %d\n", WSAGetLastError());
		}
		WSACleanup();
		exit(0);
	}
	// some error checking here

	// call cleanup when done with everything and ready to exit program
	if (closesocket(SendSocket) == SOCKET_ERROR) {
		printf("recvfrom() generated error %d\n", WSAGetLastError());
	}
	WSACleanup();

	if (RecvAddr.sin_addr.s_addr != remote.sin_addr.s_addr || RecvAddr.sin_port != remote.sin_port){
		printf("\tReply address did not match send address. Exit\n");
		exit(0);
	}

	FixedDNSheader *Recv_fdh = (FixedDNSheader*)recv_buf;
	
	if (ntohs(Recv_fdh->ID!=fdh->ID)) {
		printf("\tID did not match send address. Exit\n");
		exit(0);
	}

	int nQuestions = ntohs(Recv_fdh->nQuestions);
	int nAnswers = ntohs(Recv_fdh->nAnswers);
	int nAuthority = ntohs(Recv_fdh->nAuthority);
	int nAdditional = ntohs(Recv_fdh->nAdditional);
	int flag = ntohs(Recv_fdh->flags);
	int RRrecord = (flag & 0b1111);
		

	printf("\tTXID 0x%X, flags 0x%X, questions %d, answers %d, authority %d, additional %d\n",
		ntohs(Recv_fdh->ID), ntohs(Recv_fdh->flags), nQuestions,
		nAnswers, nAuthority,nAdditional);
	if (RRrecord != 0){
		printf("failed with Rcode = %d \n", RRrecord);
		exit(0);
	}
	else
		printf("Succeed with Rcode = 0 \n");

	int offset = sizeof(FixedDNSheader);
	u_char* recv_buf_u = (u_char*)recv_buf;
	printf("\t------------ [questions] ---------- \n");
	for (int i = 0; i < nQuestions; i++) {
		offset = printQuestion(recv_buf_u, offset,true);
		QueryHeader* QHans = (QueryHeader*)(recv_buf_u + offset);
		printf(" type %d class %d \n",ntohs(QHans->qType),ntohs(QHans->qClass));
		offset += sizeof(QueryHeader);
	}	
	printf("\t------------ [Answers] ---------- \n");
	for (int i = 0; i < nAnswers; i++) {
		offset = printQuestion(recv_buf_u, offset,true);//print name
		FixedRR* QHans = (FixedRR*)(recv_buf_u + offset);
		u_short QType = ntohs(QHans->qT);
		printQType(QType);
		offset += sizeof(FixedRR);
		if (QType == DNS_NS | QType == DNS_CNAME | QType == DNS_PTR) {
			offset = printQuestion(recv_buf_u, offset,true);
			//print name
		}
		else if (QType == DNS_A) {
			//print IP
			bool first = false;
			for (int i = 0; i < ntohs(QHans->len); i++) {
				if (first)
					printf(".");
				else
					first = true;
				printf("%d", recv_buf_u[offset]);
				offset++;
			}
		}
		else {
			offset = printQuestion(recv_buf_u, offset, false);
			printf(" Type not desired. Skip\n");
			continue;
		}
		printf(" TTL = %d \n", ntohs(QHans->TTL));
	}
	printf("\t------------ [nAuthority] ---------- \n");
	for (int i = 0; i < nAuthority; i++) {
		offset = printQuestion(recv_buf_u, offset, true);//print name
		FixedRR* QHans = (FixedRR*)(recv_buf_u + offset);
		u_short QType = ntohs(QHans->qT);
		printQType(QType);
		offset += sizeof(FixedRR);
		if (QType == DNS_NS | QType == DNS_CNAME | QType == DNS_PTR) {
			offset = printQuestion(recv_buf_u, offset, true);
			//print name
		}
		else if (QType == DNS_A) {
			//print IP
			bool first = false;
			for (int i = 0; i < ntohs(QHans->len); i++) {
				if (first)
					printf(".");
				else
					first = true;
				printf("%d", recv_buf_u[offset]);
				offset++;
			}
		}
		else {
			offset = printQuestion(recv_buf_u, offset, false);
			printf(" Type not desired. Skip\n");
			continue;
		}
		printf(" TTL = %d \n", ntohs(QHans->TTL));
	}	

	printf("\t------------ [nAdditional] ---------- \n");
	for (int i = 0; i < nAdditional; i++) {
		offset = printQuestion(recv_buf_u, offset, true);//print name
		FixedRR* QHans = (FixedRR*)(recv_buf_u + offset);
		u_short QType = ntohs(QHans->qT);
		printQType(QType);
		offset += sizeof(FixedRR);
		if (QType == DNS_NS | QType == DNS_CNAME | QType == DNS_PTR) {
			offset = printQuestion(recv_buf_u, offset, true);
			//print name
		}
		else if (QType == DNS_A) {
			//print IP
			bool first = false;
			for (int i = 0; i < ntohs(QHans->len); i++) {
				if (first)
					printf(".");
				else
					first = true;
				printf("%d", recv_buf_u[offset]);
				offset++;
			}
		}
		else {
			offset = printQuestion(recv_buf_u, offset, false);
			printf(" Type not desired. Skip\n");
			continue;
		}
		printf(" TTL = %d \n", ntohs(QHans->TTL));
	}

	//known bugs:

	//your TTL is fucked
	//handling AAAA or other unknown types

	return 0;
}