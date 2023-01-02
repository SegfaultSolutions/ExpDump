#include "PE.h"

NTSTATUS DisplayUsage(NTSTATUS status)
{
    std::cout << "\n[ExpDump]::About\n\n"
              << ""
              << " # ExpDump v1.0 -      Write any DLL's named exports to stdout or a file in python format\n"
              << " #                     as either a dictionary of lists or a single list.\n"
              << " # Copyright (C)       2022-2023 GRX78FL (at) SEGFAULT SOLUTIONS\n"
              << " # GRX78FL             https://grx78fl.github.io\n"
              << " # SEGFAULT SOLUTIONS  https://github.com/Segfault-Solutions\n"
              << "\n[ExpDump]::Usage\n\n"
              << "  PS> .\\ExpDump.exe <required: source file/directory> <optional: output file.py>\n"
              << "\n[ExpDump]::Examples\n\n"
              << " # Write a list entry for each DLL in $Env:WINDIR\\System32\\ to the specified\n"
              << " # file (non-recursive) and place it in a dictionary called 'DLLS'.\n\n"
              << "  PS> .\\ExpDump.exe \\Windows\\System32\\ $HOME\\Desktop\\DLLs.py\n\n"
              << "    DLLS = {\n        b'dll1.dll' = [\n            b'export1',\n            b'export2',\n"
              << "            ...\n        ],\n        b'dll2.dll' = [\n            ...\n    }\n\n"
              << " # Write a single list containing all the named exports specified in a DLL of choice.\n\n"
              << "  PS> .\\ExpDump.exe \\Windows\\System32\\kernel32.dll $HOME\\Desktop\\DLLS.py\n\n"
              << "    dll1 = [\n        b'export1',\n        b'export2',\n        ...\n    ]\n\n"
              << " # Not including an output file simply prints the results to the console.\n\n";
    return status;
}

int main(int argc, char* argv[])
{
    switch (argc)
    {
        case 2:
        {
            if (!lstrcmpA(argv[1], "-h"))
            {
                return DisplayUsage(EXIT_SUCCESS);
            }
            else if (PathFileExistsA(argv[1]))
            {
                PE object(argv[1], nullptr);
            }
            break;
        }
        case 3:
        {
            if (!lstrcmpA(argv[1], "-h") || !lstrcmpA(argv[2], "-h"))
            {
                return DisplayUsage(EXIT_SUCCESS);
            }
            else if (PathFileExistsA(argv[1]))
            {
                PE object(argv[1], argv[2]);
                return EXIT_SUCCESS;
            }
            break;
        }
        default:
        {
            return DisplayUsage(EXIT_FAILURE);
        }
    }
}