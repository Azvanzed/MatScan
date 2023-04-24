#ifndef H_MEMORY
#define H_MEMORY

#include <Windows.h>

#include <functional>
#include <string>

class CMemory {
 private:
  HANDLE m_Handle;

 public:
  CMemory(ULONG64 processId);
  ~CMemory();

  BOOLEAN Read(ULONG64 Address, PVOID Buffer, SIZE_T Size) const;
 
  template <typename T>
  __forceinline T Read(ULONG64 Address) const {
    T Buffer;
    Read(Address, &Buffer, sizeof(T));
    return Buffer;
  }

  VOID walkMemory(std::function<BOOLEAN(CONST MEMORY_BASIC_INFORMATION&)> pCallback) const;
  ULONG64 findModule(CONST std::wstring& moduleName) const;
};

#endif