#include "platform.h"
int
platform_is_big_endian()
{
	unsigned short v = 0x01;

	if (v == *(unsigned char*)(&v)) {
		return 0;
	} else {
		return 1;
	}
}

uint32_t
htonl(uint32_t hostlong)
{
	uint32_t tmpl;
	char *p;
	char tmpc;

	if (platform_is_big_endian()) {
		return hostlong;
	} else {
		tmpl = hostlong;
		char *p = (char*)&tmpl;

		// 0 <--> 3
		tmpc = p[0];
		p[0] = p[3];
		p[3] = tmpc;

		// 1 <-->2
		tmpc = p[1];
		p[1] = p[2];
		p[2] = tmpc;

		return tmpl;
	}
}

uint16_t
htons(uint16_t hostshort)
{
	uint16_t tmpl;
	char *p;
	char tmpc;

	if (platform_is_big_endian()) {
		return hostshort;
	} else {
		tmpl = hostshort;
		char *p = (char*)&tmpl;

		// 0 <--> 3
		tmpc = p[0];
		p[0] = p[1];
		p[1] = tmpc;
		return tmpl;
	}
}

uint32_t
ntohl(uint32_t netlong)
{
	return htonl(netlong);
}

uint16_t
ntohs(uint16_t netshort)
{
	return htons(netshort);
}



