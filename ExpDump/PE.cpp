#include "PE.h"

PE::PE(char* src, char* out)
    : _srcPath(src), _outPath(out)
{
    if (PathIsDirectoryA(_srcPath))
    {
        _IterateDirectory();
    }
    else
    {
        std::filesystem::path fileName(_srcPath);
        _expData += std::format("{} = [", fileName.stem().string());
        _LoadFile(_srcPath);
        _expData += "\n]";
    }
    _WriteData();
}

void PE::_IterateDirectory(void)
{
    DWORD _oldPathLen = GetCurrentDirectoryA(0, nullptr);
    _oldPath = new char[_oldPathLen + 1];
    memset(_oldPath, '\0', _oldPathLen + 1);
    GetCurrentDirectoryA(_oldPathLen, _oldPath);

    if ((_isDirChanged = SetCurrentDirectoryA(_srcPath)) == false)
    {
        std::cerr << std::format("[!] Couldn't set current directory to \"{}\".\n    Make sure your user has the rights to access this path.", _srcPath);
        return;
    }

    _iterating = _isDirChanged;

    WIN32_FIND_DATAA currentSearch{};
    HANDLE searchHandle = FindFirstFileA("*.dll", &currentSearch);

    if (searchHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << std::format("[!] The search for *.dll files in \"{}\" failed.\n", _srcPath);
        return;
    }
    else
    {
        _expData += "DLLS = {";
        do
        {
            _expData += std::format("\n{}b'{}' = [ ", _indent4, currentSearch.cFileName);
            _LoadFile(currentSearch.cFileName);
            _expData += std::format("\n{}],", _indent4);
        } while (FindNextFileA(searchHandle, &currentSearch));
        _expData += "\n}";
    }
    FindClose(searchHandle);
}

void PE::_LoadFile(char* fileName)
{
    HANDLE currentFileHandle = CreateFileA(fileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (currentFileHandle == INVALID_HANDLE_VALUE)
    {
        std::cerr << std::format("[!] Failed to open \"{}\" for reading, skipping...\n", fileName);
        return;
    }

    HANDLE currentFileMappingHandle = CreateFileMappingW(currentFileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (!currentFileMappingHandle)
    {
        std::cerr << std::format("[!] Failed to create a mapping for \"{}\", skipping...\n", fileName);
        CloseHandle(currentFileHandle);
        return;
    }

    LPVOID fileBase = MapViewOfFile(currentFileMappingHandle, FILE_MAP_READ, 0, 0, 0);
    if (!fileBase)
    {
        std::cerr << std::format("[!] Failed to map \"{}\", skipping...\n", fileName);
        CloseHandle(currentFileMappingHandle);
        CloseHandle(currentFileHandle);
        return;
    }

    _DumpExports(reinterpret_cast<PBYTE>(fileBase), fileName);

    UnmapViewOfFile(fileBase);
    CloseHandle(currentFileMappingHandle);
    CloseHandle(currentFileHandle);
}

void PE::_DumpExports(PBYTE base, char* fileName)
{
    _hDOS = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
    if (_hDOS->e_magic != IMAGE_DOS_SIGNATURE)
    {
        std::cerr << std::format("[!] DOS signature not found for \"{}\", skipping...\n", fileName);
        return;
    }

    _hNT = reinterpret_cast<PIMAGE_NT_HEADERS>(static_cast<PBYTE>(base) + _hDOS->e_lfanew);
    if (_hNT->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << std::format("[!] NT signature not found for \"{}\", skipping...\n", fileName);
        return;
    }

    _hFile = static_cast<PIMAGE_FILE_HEADER>(&_hNT->FileHeader);

    if (_hFile->Machine == IMAGE_FILE_MACHINE_I386)
    {
        _hOPT32 = reinterpret_cast<PIMAGE_OPTIONAL_HEADER32>(&_hNT->OptionalHeader);
        _hSect = reinterpret_cast<PIMAGE_SECTION_HEADER>(reinterpret_cast<PBYTE>(_hNT) + sizeof(IMAGE_NT_HEADERS32));
        _dataDir = &_hOPT32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    }
    else if (_hFile->Machine == IMAGE_FILE_MACHINE_AMD64)
    {
        _hOPT64 = reinterpret_cast<PIMAGE_OPTIONAL_HEADER64>(&_hNT->OptionalHeader);
        _hSect = reinterpret_cast<PIMAGE_SECTION_HEADER>(reinterpret_cast<PBYTE>(_hNT) + sizeof(IMAGE_NT_HEADERS64));
        _dataDir = &_hOPT64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    }
    else
    {
        std::cerr << std::format("[!] \"{}\" unsupported architecture, skipping...\n    Supported architectures are x86 and x64.\n", fileName);
        return;
    }

    if (_dataDir->Size == 0)
    {
        return;
    }

    if (_iterating)
    {
        _indent = _indent8;
    }
    else
    {
        _indent = _indent4;
    }

    _hExp = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(base + _RVA2Offset(_dataDir->VirtualAddress));
    DWORD* nameRvas = (DWORD*) (base + _RVA2Offset(_hExp->AddressOfNames));

    for (uint64_t i = 0; i < _hExp->NumberOfNames; i++)
    {
        auto offset = _RVA2Offset(nameRvas[i]);
        if (offset)
        {
            _expData += std::format("\n{}b'{}',", _indent, (char*) (base + offset));
        }
    }
}

void PE::_WriteData(void)
{
    if (_isDirChanged)
    {
        SetCurrentDirectoryA(_oldPath);
    }

    if (_outPath)
    {
        HANDLE outFile = CreateFileA(_outPath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (outFile == INVALID_HANDLE_VALUE)
        {
            std::cerr << std::format("[!] Couldn't create \"{}\" for writing. Aborting.", _outPath);
            return;
        }
        else
        {
            if (ERROR_ALREADY_EXISTS == GetLastError())
            {
                std::cerr << std::format("[?] \"{}\" already exists, do you want to overwrite it? [Y/N]: ", _outPath);
                char overWrite = std::cin.get();

                if (overWrite != 'y' && overWrite != 'Y')
                {
                    std::cerr << "[i] Aborting..." << std::endl;
                    CloseHandle(outFile);
                    return;
                }
            }

            DWORD nBytesToWrite = static_cast<DWORD>(_expData.length());
            DWORD nBytesWritten = 0;

            WriteFile(outFile, _expData.c_str(), nBytesToWrite, &nBytesWritten, nullptr);

            if (nBytesToWrite != nBytesWritten)
            {
                std::cerr << std::format("[!] Couldn't write export data to \"{}\"", _outPath);
                CloseHandle(outFile);
                return;
            }
            SetEndOfFile(outFile);
        }
    }
    else
    {
        std::cout << _expData;
    }
}

DWORD PE::_RVA2Offset(DWORD rva)
{
    for (uint16_t i = 0; i <= _hFile->NumberOfSections; i++)
    {
        if (rva >= _hSect[i].VirtualAddress)
        {
            if (rva < (_hSect[i].VirtualAddress + _hSect[i].SizeOfRawData))
            {
                return rva - _hSect[i].VirtualAddress + _hSect[i].PointerToRawData;
            }
        }
    }
    return 0;
}

PE::~PE(void)
{
    if (_oldPath)
    {
        delete[] _oldPath;
    }
}