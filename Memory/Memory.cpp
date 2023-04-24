#include <Memory/Memory.hpp>
#include <Undocumented.hpp>

CMemory::CMemory(ULONG64 processId)
    : m_Handle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId)) {}

CMemory::~CMemory() { /*if (m_Handle) CloseHandle(m_Handle);*/ }

BOOLEAN CMemory::Read(ULONG64 Address, PVOID Buffer, SIZE_T Size) const {
  SIZE_T readBytes;
  return ReadProcessMemory(m_Handle, (PVOID)Address, Buffer, Size, &readBytes) &&
      readBytes == Size;
}

VOID CMemory::walkMemory(
    std::function<BOOLEAN(CONST MEMORY_BASIC_INFORMATION&)> pCallback) const {
  MEMORY_BASIC_INFORMATION memInfo;
  memset(&memInfo, 0, sizeof(MEMORY_BASIC_INFORMATION));

  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);

  for (ULONG64 Curr = (ULONG64)sysInfo.lpMinimumApplicationAddress;
       Curr < (ULONG64)sysInfo.lpMaximumApplicationAddress;
       Curr = (ULONG64)memInfo.BaseAddress + memInfo.RegionSize) {
    if (!VirtualQueryEx(m_Handle, (PVOID)Curr, &memInfo,
                        sizeof(MEMORY_BASIC_INFORMATION)) ||
        !pCallback(memInfo))
      continue;

    break;
  }
}

ULONG64 CMemory::findModule(CONST std::wstring& moduleName) const {
  ULONG64 modBase = 0;
  walkMemory([&](CONST MEMORY_BASIC_INFORMATION& memInfo) -> BOOLEAN {
    BYTE Data[1024];
    SIZE_T readBytes;
    if (!NT_SUCCESS(NtQueryVirtualMemory(m_Handle, memInfo.BaseAddress,
                                         MemoryMappedFilenameInformation, Data,
                                         sizeof(Data), &readBytes)))
      return FALSE;

    std::wstring fileName = std::wstring(((UNICODE_STRING*)Data)->Buffer);
    if (fileName.find(moduleName) == std::wstring::npos ||
        fileName.find(L".mui") != std::wstring::npos)
      return FALSE;

    modBase = (ULONG64)memInfo.BaseAddress;
    return TRUE;
  });

  return modBase;
}