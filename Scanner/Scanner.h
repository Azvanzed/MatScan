#ifndef H_SCANNER
#define H_SCANNER

#include <Windows.h>

#include <Memory/Memory.hpp>
#include <vector>
#include <mutex>

namespace Scanner {
inline SYSTEM_INFO g_SysInfo;

typedef struct _MATERIAL // haha windows style structs!!!
{
  CHAR Name[129];
  std::vector<ULONG> Chain;
}MATERIAL, *PMATERIAL;

inline std::vector<MATERIAL> g_foundMaterials;
inline std::mutex g_Mutex;

BOOLEAN isValid(ULONG64 Address);

VOID scanThread(CMemory rustClient, ULONG64 classBase, SHORT Range, ULONG Depth,
                CONST std::vector<ULONG>& Start = {},
                CONST std::vector<ULONG>& Chain = {});
}  // namespace Scanner

#endif