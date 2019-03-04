#pragma once
#include "stdafx.h"

class Socket {
	SOCKET sock; // socket handle
	int allocatedSize; // bytes allocated for buf
	int curPos; // current position in buffer
public:
	char *buf = NULL; // current buffer
	Socket(void) {};
	//bool Read(myURL url);
	~Socket() {
		if (buf != NULL)
			delete buf;
	};
};

#pragma pack(push,1)
struct QueryHeader {
	USHORT qType;
	USHORT qClass;
};

struct FixedDNSheader {
	USHORT ID;
	USHORT flags;
	USHORT nQuestions;
	USHORT nAnswers;
	USHORT nAuthority;
	USHORT nAdditional;
};

class DNSanswerHdr {
	USHORT type;
	USHORT DNSclass;
	USHORT ttl;
	USHORT len;
};
#pragma pack(pop)