#include <Scanner/Scanner.h>

BOOLEAN Scanner::isValid(ULONG64 Address) {
  return Address >= (ULONG64)g_SysInfo.lpMinimumApplicationAddress &&
         Address <= (ULONG64)g_SysInfo.lpMaximumApplicationAddress;
}

VOID Scanner::scanThread(
    CMemory rustClient, ULONG64 classBase, SHORT Range, ULONG Depth,
    CONST std::vector<ULONG>& Start,
               CONST std::vector<ULONG>& Chain) {
  for (SIZE_T Offset = 0; Offset < Range; Offset += sizeof(ULONG64)) {
    ULONG64 Value = rustClient.Read<ULONG64>(classBase + Offset);
    if (!isValid(Value)) continue;

    ULONG64 Material = rustClient.Read<ULONG64>(Value + 0x10);
    if (!isValid(Material)) continue;

    if (rustClient.Read<ULONG64>(Material + 0x88) == 16045690984833335023) {
      ULONG64 Name = rustClient.Read<ULONG64>(Material + 0x30);
      if (!isValid(Name)) continue;

      CHAR Buffer[129];
      rustClient.Read(Name, Buffer, sizeof(Buffer));
      Buffer[128] = '\0';

      MATERIAL Material;
      strcpy(Material.Name, Buffer);

      Material.Chain.insert(Material.Chain.begin(), Start.begin(), Start.end());
      Material.Chain.insert(Material.Chain.begin() + Start.size(), Chain.begin(),
                            Chain.end());
      Material.Chain.push_back(Offset);

      g_Mutex.lock();
     g_foundMaterials.push_back(Material);
      g_Mutex.unlock();

      printf("%s: [ ", Buffer);
      for (SIZE_T i = 0; i < Material.Chain.size(); ++i)
        printf("0x%02x%s", Material.Chain.at(i),
               i != Material.Chain.size() - 1 ? ", " : " ");
      printf("]\n");


      continue;
    }

    if (Depth) {
      std::vector<ULONG> newChain;
      newChain.push_back(Offset);

      scanThread(rustClient, Value, Range, Depth - 1, Start, newChain);
    }
  }
}