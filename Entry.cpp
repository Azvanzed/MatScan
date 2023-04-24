#include <Scanner/Scanner.h>
#include <Windows.h>

#include <Memory/Memory.hpp>
#include <Utils/Utils.hpp>
#include <fstream>
#include <json.hpp>
#include <thread>

// i know i made some mistakes, let me know if you find one

VOID Helper() { printf("Usage: ScriptFile ScanRange ScanDepth ThreadCount\nEx: script.json 1000 2 4"); }

LONG main(LONG Argc, CHAR* Argv[]) {
  
   if (Argc < 5) Helper();

   CHAR* scriptFile = Argv[1];
  LONG scanRange = std::atoi(Argv[2]);
  LONG scanDepth = std::atoi(Argv[3]);
  LONG threadCount = std::atoi(Argv[4]);

  printf("Settings:\n");
  printf("scriptFile: %s\n", scriptFile);
  printf("scanRange: %i\n", scanRange);
  printf("scanDepth: %i\n", scanDepth);
  printf("threadCount: %i\n\n", threadCount);

  printf("Gathering offsets...\n");
  std::vector<ULONG> Offsets;
  {
    std::ifstream inputFile{scriptFile};
    if (!inputFile.is_open())
    {
      printf("Failed to open %s", scriptFile);
      return 0;
    }

    std::string Buffer, Line;
    while (std::getline(inputFile, Line)) Buffer.append(Line);

    inputFile.close();

    nlohmann::json scriptMetaData =
        nlohmann::json::parse(Buffer)["ScriptMetadata"];

    for (auto& metaData : scriptMetaData.items())
    {
      auto& Value = metaData.value();
      if (Value.find("Address") == Value.end())
        continue;

      Offsets.push_back(Value["Address"]);
    }
  }

  if (Offsets.empty())
  {
    printf("Couldn't extract offsets\n");
    return 0;
  }

  ULONG64 processId = Utils::findProcessId(L"RustClient.exe");
  if (!processId)
  {
    printf("Rust must be open (join a server)\n");
    return 0;
  }

  printf("RustClient.exe: 0x%llx\n", processId);

  CMemory rustClient{Utils::findProcessId(L"RustClient.exe")};
  ULONG64 gameAssembly = rustClient.findModule(L"GameAssembly.dll");
  printf("GameAssembly.dll: 0x%llx\n", gameAssembly);

 

  /* This is just a quick multi threading nothing fancy */

  printf("Starting %i thread(s)\n", threadCount);
  GetSystemInfo(&Scanner::g_SysInfo);

  SIZE_T offsetCount = Offsets.size();

  std::atomic<LONG> freeThreadCount = threadCount; // avoid da racing
  LONG chunkPerThread = offsetCount / freeThreadCount; 
  
  for (LONG currThreadIdx = 0; currThreadIdx < threadCount; ++currThreadIdx) {
    std::thread([Offsets, chunkPerThread, currThreadIdx, threadCount, offsetCount,
                 &freeThreadCount, gameAssembly, rustClient, scanRange,
                 scanDepth]() {
      LONG chunkSize =
          chunkPerThread +
          (currThreadIdx == threadCount - 1 ? offsetCount % threadCount : 0);
      printf("Started thread %i searching %i classes\n", currThreadIdx + 1,
             chunkSize);

      --freeThreadCount;

      for (ULONG currOffset = 0; currOffset < chunkSize; ++currOffset) {
        ULONG Offset = Offsets[chunkPerThread * currThreadIdx + currOffset];

        ULONG64 Class = rustClient.Read<ULONG64>(
            gameAssembly +
            Offsets[chunkPerThread * currThreadIdx + currOffset]);
        if (!Scanner::isValid(Class)) continue;

        ULONG64 staticField = rustClient.Read<ULONG64>(Class + 0xB8);
        if (!Scanner::isValid(staticField)) continue;

        Scanner::scanThread(rustClient, staticField, scanRange, scanDepth,
                            {Offset, 0xB8});
      }

      ++freeThreadCount;
      printf("Thread %i finished searching\n", currThreadIdx + 1);
    }).detach();
  }

  do {
    Sleep(100);

    std::string conTitle =
        "Working thread(s): " + std::to_string(threadCount - freeThreadCount) + "/" +
        std::to_string(threadCount);

    SetConsoleTitleA(conTitle.c_str());
  }
  while (freeThreadCount != threadCount);

  printf("Scan done, saving...\n");

  std::ofstream outputFile{"Chains.txt"};
  for (SIZE_T i = 0; i < Scanner::g_foundMaterials.size();
       ++i) {
    CONST Scanner::MATERIAL& Material = Scanner::g_foundMaterials[i];

    outputFile << Material.Name << ": [ ";
    for (SIZE_T i = 0; i < Material.Chain.size(); ++i) {
      outputFile << "0x" << std::hex << Material.Chain.at(i)
                 << (i != Material.Chain.size() - 1 ? ", " : " ");
    }
    outputFile << "]\n";
  }
  outputFile.close();
  return 0;
}