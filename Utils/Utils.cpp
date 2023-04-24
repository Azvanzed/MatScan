#include <Utils/Utils.hpp>
#include <Undocumented.hpp>

#include <TlHelp32.h>

ULONG64 Utils::findProcessId(CONST std::wstring& processName)
{
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Snapshot == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32 processEntry;
	memset(&processEntry, 0, sizeof(PROCESSENTRY32));
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	Process32First(Snapshot, &processEntry);
	do
	{
		if (processName.find(processEntry.szExeFile) == std::string::npos)
			continue;

		return processEntry.th32ProcessID;
	} while (Process32Next(Snapshot, &processEntry));

	return 0;
}