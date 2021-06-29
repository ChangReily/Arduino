# Arduino
* VS Code Envoirment setting
```
.vscode\arduino.json
{
    "sketch": "app.ino",
    "board": "arduino:avr:uno",
    "output": "build",
    "programmer": "AVR ISP"
}

.vscode\c_cpp_properties.json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${default}",
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\**",
                "C:\\Program Files (x86)\\Arduino\\tools\\**"
            ],
            "forcedInclude": [
                "C:\\Program Files (x86)\\Arduino\\hardware\\arduino\\avr\\cores\\arduino\\Arduino.h"
            ],
            "intelliSenseMode": "msvc-x64",
            "compilerPath": "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/bin/cl.exe",
            "cStandard": "c11",
            "cppStandard": "c++17"
        }
    ],
    "version": 4
}
```
