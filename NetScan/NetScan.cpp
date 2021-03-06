#include "NetScan.h"
#include "winapi/winapi.h"
#include "winapi/listview.h"

#include <stdio.h>
#include <assert.h>


#include "Win32/resource.h"

#include "winapi/IpHelp.hpp"
#include <IcmpAPI.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


void DebugPrintMAC(PBYTE const data, ULONG len)
{
	if (!len)
		goto done;
start:
	printf("%x",*data);
	if (!--len)
		goto done;
	printf(":");
	goto start;
done:
	;
}

void DebugPrint(PIP_ADAPTER_ADDRESSES adapter)
{
	printf("\n*Adapter %S*\n",adapter->FriendlyName);
	printf("IfIndex: %d\n", adapter->IfIndex);
	printf("AdapterName: %s\n", adapter->AdapterName);
	for (auto address = adapter->FirstUnicastAddress; address; address = address->Next)
	{
		printf("Unicast: \n");
	}
	for (auto address = adapter->FirstAnycastAddress; address; address = address->Next)
	{
		printf("Anycast: \n");
	}
	for (auto address = adapter->FirstMulticastAddress; address; address = address->Next)
	{
		printf("Multicast: \n");
	}
	for (auto address = adapter->FirstDnsServerAddress; address; address = address->Next)
	{
		printf("DnsServer: \n");
	}
	printf("DnsSuffix: %S\n", adapter->DnsSuffix);
	printf("Description: %S\n", adapter->Description);
	if (adapter->PhysicalAddressLength) {
		printf("MAC: ");
		DebugPrintMAC(adapter->PhysicalAddress, adapter->PhysicalAddressLength);
		printf("\n");
	}
	printf("Flags: 0x%08x\n", adapter->Flags);
	printf("Mtu: %d\n", adapter->Mtu);
	printf("IfType: %s\n", InterfaceType(adapter->IfType).ToString());
}

void DebugPrint(PIP_ADAPTER_INFO info)
{
	printf("ComboIndex:  %d\n", info->ComboIndex);
	printf("AdapterName: %s\n", info->AdapterName);
	printf("Description: %s\n", info->Description);
	if (info->AddressLength)
	{
		printf("MAC:       ");
		DebugPrintMAC(info->Address, info->AddressLength);
		printf("\n");
	}
}

NetScan::NetScan()
{
	WSADATA wd;
	WSAStartup(0x0202, &wd);
}

void NetScan::networks1()
{
	char name[256] = { 0 };
	int err = gethostname(name, sizeof(name));

	hostent* info = gethostbyname(NULL); // "" or name should return the same thing.

	printf("addrtype: %s\n", AddrType(info->h_addrtype).ToString());

	struct in_addr addr;

	if (info->h_addrtype == AF_INET) {
		for (int i = 0; info->h_addr_list[i] != 0;)
		{
			addr.s_addr = *(u_long *)info->h_addr_list[i++];
			printf("\tIPv4 Address #%d: %s\n", i, inet_ntoa(addr));
		}
	}
	printf("\n");
}

void NetScan::networks2()
{
	ULONG Size = 0x4000;
	PIP_ADAPTER_INFO AdapterInfo = (PIP_ADAPTER_INFO)malloc(Size);
	GetAdaptersInfo(AdapterInfo, &Size);
	for (auto adapter = AdapterInfo; adapter; adapter = adapter->Next)
	{
		DebugPrint(AdapterInfo);
		printf("\n");
	}
}

void NetScan::networks3()
{
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
	flags |= GAA_FLAG_INCLUDE_ALL_INTERFACES;
	USHORT family = AF_UNSPEC;

	ULONG len = 0x10000;
	PIP_ADAPTER_ADDRESSES AdapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(len);
	//ULONG len = sizeof(AdapterAddresses);

	GetAdaptersAddresses(family, flags, NULL, AdapterAddresses,&len);
	for (auto adapter = AdapterAddresses; adapter; adapter = adapter->Next)
	{
		DebugPrint(adapter);
	}


/*
struct ifaddrs *ifap, *ifa;
struct sockaddr_in *sa;
char *addr;

getifaddrs(&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_netmask;
			addr = inet_ntoa(sa->sin_addr);
			printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
		}
	}

	freeifaddrs(ifap);*/

	return;
}


template<typename lambda_t>
void NetScan::EnumIpTable(lambda_t lambda)
{
	ULONG Size = 0;
	DWORD err = GetIpNetTable(NULL, &Size, FALSE);
	assert(err == ERROR_INSUFFICIENT_BUFFER);
	assert(Size && Size < 0x1000);
	PMIB_IPNETTABLE IpNetTable = (PMIB_IPNETTABLE)malloc(Size);
	err = GetIpNetTable(IpNetTable, &Size, FALSE);
	assert(!err);
	for (DWORD i = 0; i < IpNetTable->dwNumEntries; i++)
	{
		PMIB_IPNETROW entry = &IpNetTable->table[i];
		lambda(entry);
		continue;
	}
	free(IpNetTable);
}


int NetScan::scan(char const* address)
{
	// Declare and initialize variables

	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	DWORD dwRetVal = 0;
	DWORD dwError = 0;
	char SendData[] = "Data Buffer";
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;

	ipaddr = inet_addr(address);
	if (ipaddr == INADDR_NONE) {
		printf("usage: NetScan.exe <IP address>\n");
		return 1;
	}

	hIcmpFile = IcmpCreateFile();
	if (hIcmpFile == INVALID_HANDLE_VALUE) {
		printf("\tUnable to open handle.\n");
		printf("IcmpCreatefile returned error: %ld\n", GetLastError());
		return 1;
	}
	// Allocate space for at a single reply
	ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData) + 8;
	ReplyBuffer = (VOID *)malloc(ReplySize);
	if (ReplyBuffer == NULL) {
		printf("\tUnable to allocate memory for reply buffer\n");
		return 1;
	}

	dwRetVal = IcmpSendEcho2(hIcmpFile, NULL, NULL, NULL,
		ipaddr, SendData, sizeof(SendData), NULL,
		ReplyBuffer, ReplySize, 1000);
	if (dwRetVal != 0) {
		PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
		struct in_addr ReplyAddr;
		ReplyAddr.S_un.S_addr = pEchoReply->Address;
		printf("\tSent icmp message to %s\n", address);
		if (dwRetVal > 1) {
			printf("\tReceived %ld icmp message responses\n", dwRetVal);
			printf("\tInformation from the first response:\n");
		}
		else {
			printf("\tReceived %ld icmp message response\n", dwRetVal);
			printf("\tInformation from this response:\n");
		}
		printf("\t  Received from %s\n", inet_ntoa(ReplyAddr));
		printf("\t  Status = %ld  ", pEchoReply->Status);
		switch (pEchoReply->Status) {
		case IP_DEST_HOST_UNREACHABLE:
			printf("(Destination host was unreachable)\n");
			break;
		case IP_DEST_NET_UNREACHABLE:
			printf("(Destination Network was unreachable)\n");
			break;
		case IP_REQ_TIMED_OUT:
			printf("(Request timed out)\n");
			break;
		default:
			printf("\n");
			break;
		}

		printf("\t  Roundtrip time = %ld milliseconds\n",
			pEchoReply->RoundTripTime);
	}
	else {
		printf("Call to IcmpSendEcho2 failed.\n");
		dwError = GetLastError();
		switch (dwError) {
		case IP_BUF_TOO_SMALL:
			printf("\tReplyBufferSize to small\n");
			break;
		case IP_REQ_TIMED_OUT:
			printf("\tRequest timed out\n");
			break;
		default:
			printf("\tExtended error returned: %ld\n", dwError);
			break;
		}
		return 1;
	}
	return 0;

}

void print_MIB_IPADDR_wtype(USHORT wType)
{
	char const* sep = "";
	if (wType & MIB_IPADDR_PRIMARY) {
		printf("MIB_IPADDR_PRIMARY"); sep = "|";
	}
	if (wType & MIB_IPADDR_DYNAMIC) {
		printf("%sMIB_IPADDR_DYNAMIC",sep); sep = "|";
	}
	if (wType & MIB_IPADDR_DISCONNECTED) {
		printf("%sMIB_IPADDR_DISCONNECTED", sep); sep = "|";
	}
	if (wType & MIB_IPADDR_DELETED) {
		printf("%sMIB_IPADDR_DELETED", sep); sep = "|";
	}
	if (wType & MIB_IPADDR_TRANSIENT) {
		printf("%sMIB_IPADDR_TRANSIENT", sep); sep = "|";
	}
	if (wType & MIB_IPADDR_DNS_ELIGIBLE) {
		printf("%sMIB_IPADDR_DNS_ELIGIBLE", sep);
	}
}

void print_MAC(PUCHAR const data, DWORD len)
{
	if (!len)
		goto done;
start:
	printf("%x", *data);
	if (!--len)
		goto done;
	printf(":");
	goto start;
done:
	;
}

/*
using ListBox = class win32::ListBox;
template<class t> 
using Scoped = class win32::Scoped<t>;
using Convert = class win32::Convert;
using Dialog = class win32::dialog;
using String = class win32::String;
using Window = class win32::Window;
*/
using namespace win32;




int main()
{
	NetScan scanner;
	
	IpHelpers::Interfaces([](MIB_IPADDRROW& row, MIB_IFROW& row2) {
		printf("Interface: %s\n", row2.bDescr);
		printf("\taddress: %s\n", IPv4Address(row.dwAddr).ToString());
		printf("\tindex:   %d\n", row.dwIndex);
		printf("\tmask:    %s\n", IPv4Address(row.dwMask).ToString());
		printf("\tbcast:   %s\n", IPv4Address(row.dwBCastAddr).ToString());
		printf("\treasm:   %d\n", row.dwReasmSize);
		printf("\ttype:    ");
		print_MIB_IPADDR_wtype(row.wType);
		printf("\n");
	});

	printf("%s\t%-15s\t%-20s\t%s\n", "If", "Ip Address", "MAC Address", "Type");
	scanner.EnumIpTable([](PMIB_IPNETROW row) {
		const char* type[5] = { "-","other", "invalid", "dynamic", "static" };
		printf("%3d\t%-15s %-20s\t%s\n",
			row->dwIndex,
			IPv4Address(row->dwAddr).ToString(),
			PhysicalAddress(row->bPhysAddr, row->dwPhysAddrLen).ToString(),
			type[row->dwType]);
	});
	

	/*
	auto dlg = Dialog::createFromResource(NULL, IDD_INTERFACES);
	Scoped<Dialog> x(dlg);

	IpHelpers::Interfaces([=](MIB_IPADDRROW& row, MIB_IFROW& row2) {
		dlg->Control<ListBox>(IDC_INTERFACES).AddString(*Convert::ToString(row2.bDescr));
	});

	dlg->show();
	*/

	WindowClass mainWindowClass(GetModuleHandle(NULL), TEXT("MainWindow"), [](WindowClassBuilder& builder) {
		builder
			.withBackground(COLOR_3DFACE);
	});

	auto mainWindow = mainWindowClass.Create<Window>([](WindowCreateBuilder& builder) {
		builder.asOverlappedWindow()
			.withTitle(TEXT("MainWindow"));
	});

	auto list = ListView::Create(GetModuleHandle(NULL), *mainWindow, [&](WindowCreateBuilder& builder) {
		
		RECT rc;
		GetClientRect(**mainWindow, &rc);
		builder.withStyle(LVS_REPORT | LVS_EDITLABELS)
			.atPoint({ 0, 0 })
			.withSize({ rc.right - rc.left, rc.bottom - rc.top });
	});

	list->Column().Add(0).withWidth(280).withLabel(TEXT("Description")).alignLeft();
	list->Column().Add(1).withWidth( 50).withLabel(TEXT("Index")).alignRight();
	list->Column().Add(2).withWidth(100).withLabel(TEXT("Address")).alignRight();
	list->Column().Add(3).withWidth(100).withLabel(TEXT("Mask")).alignRight();
	list->Column().Add(4).withWidth(100).withLabel(TEXT("Broadcast")).alignRight();
	list->Column().Add(5).withWidth(100).withLabel(TEXT("Reasm")).alignRight();
	list->Column().Add(6).withWidth(200).withLabel(TEXT("Type")).alignLeft();

	IpHelpers::Interfaces([&](MIB_IPADDRROW& row, MIB_IFROW& row2) {
		list->Item().Insert(0).withText(*Convert::ToString(row2.bDescr));
		list->Item(0).setText(1, *Convert::ToString(row.dwIndex));
		list->Item(0).setText(2, *Convert::ToString(IPv4Address(row.dwAddr).ToString()));
		list->Item(0).setText(3, *Convert::ToString(IPv4Address(row.dwMask).ToString()));
		list->Item(0).setText(4, *Convert::ToString(IPv4Address(row.dwBCastAddr).ToString()));
		list->Item(0).setText(5, *Convert::ToString(row.dwReasmSize));
	});

	mainWindow->show();

	return win32::Application::Run();
}
