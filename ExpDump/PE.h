#pragma once

#include <Windows.h>
#include <Shlwapi.h>

#include <format>
#include <iostream>
#include <filesystem>

class PE
{
public:
    PE(char* src, char* out);
    ~PE(void);

private:
    void _IterateDirectory(void);
    void _LoadFile(char* fileName);
    void _DumpExports(PBYTE base, char* fileName);
    void _WriteData(void);
    DWORD _RVA2Offset(DWORD rva);
    PE(void){};

private:
    char* _srcPath = nullptr;
    char* _outPath = nullptr;
    char* _oldPath = nullptr;
    bool _isDirChanged = false;
    bool _iterating = false;
    PIMAGE_DOS_HEADER _hDOS = nullptr;
    PIMAGE_NT_HEADERS _hNT = nullptr;
    PIMAGE_FILE_HEADER _hFile = nullptr;
    PIMAGE_OPTIONAL_HEADER32 _hOPT32 = nullptr;
    PIMAGE_OPTIONAL_HEADER64 _hOPT64 = nullptr;
    PIMAGE_DATA_DIRECTORY _dataDir = nullptr;
    PIMAGE_EXPORT_DIRECTORY _hExp = nullptr;
    PIMAGE_SECTION_HEADER _hSect = nullptr;
    std::string _expData;
    std::string _indent;
    const char _indent4[5] = "    ";
    const char _indent8[9] = "        ";
};
