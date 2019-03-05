#pragma once
#include "stdafx.h"

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

struct DNSanswerHdr {
	USHORT qT;
	USHORT qC;
	USHORT TTL;
	USHORT len;
};

struct FixedRR {
	u_short qT;
	u_short qC;
	int TTL;
	u_short len;
};
#pragma pack(pop)