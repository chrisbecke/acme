#pragma once

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


class NetScan
{
public:
	NetScan();
	int scan(char const* argv);
	void networks1();
	void networks2();
	void networks3();

	template<typename lambda_t>
	void EnumIpTable(lambda_t lambda);
};
