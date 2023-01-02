### **[ExpDump]::Usage**
```powershell
PS> .\ExpDump.exe <required: source file/directory> <optional: output file.py>
```

### **[ExpDump]::Examples**
##### Command:
```powershell
# Write a list entry for each DLL in $Env:WINDIR\System32\ to the specified
# file (non-recursive) and place it in a dictionary called 'DLLS'.
PS> .\ExpDump.exe \Windows\System32\ $HOME\Desktop\DLLs.py
```
##### Output:
```python
DLLS = {
    b'dll1.dll' = [
        b'export1',
        b'export2',
        ...
    ],
    b'dll2.dll' = [
        ...
    ],
    ...
```
##### Command:
```powershell
# Write a single list containing all the named exports specified in a DLL of choice.
PS> .\ExpDump.exe \Windows\System32\kernel32.dll $HOME\Desktop\DLLS.py
```
##### Output:
```python
dll1 = [
    b'export1',
    b'export2',
    ...
]
```
### **NOTE**
*Not including an output file simply prints the results to the console.*
