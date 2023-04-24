#ifndef H_UTILS
#define H_UTILS

#include <iostream>
#include <Windows.h>
#include <algorithm>

namespace Utils
{
	ULONG64 findProcessId(CONST std::wstring& processName);
}

#endif H_UTILS