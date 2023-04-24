#ifndef H_UNDOCUMENTED
#define H_UNDOCUMENTED

#include <winternl.h>
#include <Windows.h>

typedef enum _MEMORY_INFORMATION_CLASS {
  MemoryBasicInformation,
  MemoryWorkingSetInformation,
  MemoryMappedFilenameInformation,
  MemoryRegionInformation,
  MemorySectionInformation
} MEMORY_INFORMATION_CLASS;

extern "C" {
NTSTATUS NtQueryVirtualMemory(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID,
                              SIZE_T, SIZE_T*);
}

#endif