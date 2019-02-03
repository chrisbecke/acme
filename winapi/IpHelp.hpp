#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

class IPv4Address
{
	in_addr in;
public:
	IPv4Address(ULONG dw) { in.S_un.S_addr = dw; }
	char const* ToString() { return inet_ntoa(in); }
};

class PhysicalAddress
{
	PUCHAR physical;
	DWORD physicalLen;
public:
	PhysicalAddress(PUCHAR data, DWORD len) { physical = data; physicalLen = len; }
	static char hex(int c)
	{
		return c < 10 ? '0' + c : 'A' + c - 10;
	}
	char const* ToString() {
		static char buffer[256];
		char* write = buffer;
		PUCHAR data = physical;
		DWORD len = physicalLen;

		if (!len)
			goto done;
		goto start;
	loop:
		*write++ = ':';
	start:
		*write++ = hex(*data >> 4 & 0x0f);
		*write++ = hex(*data & 0x0f);
		len--; data++;
		if (len)
			goto loop;
	done:
		*write = 0;
		return buffer;
	}

};

class AddrType
{
	short h_addrtype;
public:
	AddrType(short t) {
		h_addrtype = t;
	}

	char const* ToString()
	{
		switch (h_addrtype) {
		case AF_INET: return "AF_INET"; break;
		case AF_INET6: return "AF_INET6"; break;
		case AF_NETBIOS: return "AF_NETBIOS"; break;
		}
		return "(AF_xxxx)";
	}
};

class InterfaceType
{
	IFTYPE IfType;
public:
	InterfaceType(IFTYPE t) { IfType = t; }
	char const* ToString()
	{
		switch (IfType)
		{
		case IF_TYPE_OTHER:
			return "IF_TYPE_OTHER";
		case  IF_TYPE_ETHERNET_CSMACD:
			return "IF_TYPE_ETHERNET_CSMACD";
		case IF_TYPE_PPP:
			return "IF_TYPE_PPP";
		case IF_TYPE_SOFTWARE_LOOPBACK:
			return "IF_TYPE_SOFTWARE_LOOPBACK";
		case IF_TYPE_TUNNEL:
			return "IF_TYPE_TUNNEL";
		}
		return "(IF_TYPE_UNKNOWN)";
	}
};

class IpHelpers
{
public:
	template<typename lambda_t>
	static void Interfaces(lambda_t lambda)
	{
		DWORD err;
		ULONG Size;

		/*
		DWORD num = 0;
		err = GetNumberOfInterfaces(&num);

		for (DWORD i = 0; i < num; i++)
		{
			MIB_IFROW row = { 0 };
			row.dwIndex = i;
			err = GetIfEntry(&row);
			continue;
		}
		*/

		/*
		Size = 0;
		err = GetIfTable(NULL, &Size, FALSE);
		if (err != ERROR_INSUFFICIENT_BUFFER)
		{
			return;
		}
		PMIB_IFTABLE IfTable = (PMIB_IFTABLE)new char[Size];
		err = GetIfTable(IfTable, &Size, FALSE);
		for (DWORD i = 0; i < IfTable->dwNumEntries; i++)
		{
			auto entry = &IfTable->table[i];
			continue;
		}
		delete[] IfTable;
		*/

		Size = 0;
		err = GetIpAddrTable(NULL, &Size, FALSE);
		if (err != ERROR_INSUFFICIENT_BUFFER)
		{
			LPTSTR message = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&message, 0, NULL);
//			printf("%S", message);
			LocalFree(message);
			return;
		}

		PMIB_IPADDRTABLE AddrTable = (PMIB_IPADDRTABLE)new char[Size];
		err = GetIpAddrTable(AddrTable, &Size, FALSE);
		if (err)
		{
			return;
		}

		for (DWORD i = 0; i < AddrTable->dwNumEntries; i++)
		{
			MIB_IFROW row = { 0 };
			row.dwIndex = AddrTable->table[i].dwIndex;
			err = GetIfEntry(&row);
			lambda(AddrTable->table[i], row);
		}
		delete[] AddrTable;

	}
};
