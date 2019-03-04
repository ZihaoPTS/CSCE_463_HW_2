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
			printf("buf:%s", buf);
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
		//Query Type PTR
		QueryType = DNS_PTR;
	}
	else
	{
		//Is a IP
		//Query Type A
		QueryType = DNS_A;
	}

	SOCKET SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// handle errors
	if (SendSocket == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		exit(0);
	}

	int pkt_size = strlen(argv[1]) + 2 + sizeof(FixedDNSheader) + sizeof(QueryHeader);

	char *buf = new char[pkt_size];

	FixedDNSheader *fdh = (FixedDNSheader *)buf;
	QueryHeader *qh = (QueryHeader*)(buf + pkt_size - sizeof(QueryHeader));

	// fixed field initialization
	fdh->ID = 0;
	fdh->flags = htons(DNS_QUERY | DNS_RD | DNS_STDQUERY);
	fdh->nAdditional = 0;
	fdh->nAnswers = 0;
	fdh->nAuthority = 0;
	fdh->nQuestions = htons(1);
	qh->qType = htons(QueryType);
	qh->qClass = htons(DNS_INET);


	//if(DNS_PTR == DNS_PTR) then do following
	//else do reverse look up
	makeDNSquestion((char*)(fdh + 1), argv[1]);

	struct sockaddr_in local;
	int localAddrSize = sizeof(local);
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(0);
	if (bind(SendSocket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
		printf("bind() generated error %d\n", WSAGetLastError());
		exit(0);
	}

	struct sockaddr_in remote;
	memset(&remote, 0, sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(argv[2]); // server
	remote.sin_port = htons(53);
	if (sendto(SendSocket, buf, pkt_size, 0, (struct sockaddr*)&remote, sizeof(remote)) == SOCKET_ERROR) {
		printf("sendto() generated error %d\n", WSAGetLastError());
		exit(0);
	}

	char* recv_buf = new char[MAX_DNS_LEN];
	int count = 0;
	while (count++ < MAX_ATTEMPTS)
	{
		int ret;
		timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		// get ready to receive
		fd_set fd;
		FD_ZERO(&fd); // clear the set
		FD_SET(SendSocket, &fd); // add your socket to the set
		int available = select(0, &fd, NULL, NULL, &timeout);
		if (available > 0)
		{
			if (recvfrom(SendSocket, recv_buf, MAX_DNS_LEN, 0, (SOCKADDR *)& local, &localAddrSize) == SOCKET_ERROR) {
				printf("recvfrom() generated error %d\n", WSAGetLastError());
				exit(0);
			}
			// parse the response
			//break from the loop
		}
	}
	// some error checking here

	if (closesocket(SendSocket) == SOCKET_ERROR) {
		printf("recvfrom() generated error %d\n", WSAGetLastError());
		exit(0);
	}

	printf("recv_buf: %s", recv_buf);
	// call cleanup when done with everything and ready to exit program
	WSACleanup();

	return 0;
}