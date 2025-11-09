# Diff-Check-AI

1. Go to settings and ensure the Temp and Backup folder are correct. 
Browse to CMakeLists.txt, and hit load. 
This will delete the Tempfolder ProjectName_Temp, and copy all the CMake file and folder structure.
This gives you a temp project to work on.
2. Click on Open, and locate the file you want to work on in the Temp folder.
Paste in the new code you want to compare. 
Click on the menu Compare.

Command to run app:
 ```
cd "/mnt/c/Users/jflesher/OneDrive/Documents/WorkSpace/Diff-Check-AI/Qt" \&\& chmod +x src2txt.sh \&\&  dos2unix src2txt.sh \&\&  shellcheck src2txt.sh \&\& ./src2txt.sh --project "Diff-Check-AI"
```


Translations update:
```
lupdate.exe ./src -ts translations/Diff-Check-AI\_en.ts translations/Diff-Check-AI\_es.ts
lrelease.exe translations/Diff-Check-AI\_en.ts translations/Diff-Check-AI\_es.ts
```

## Build:
Debug:
```
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -G "Visual Studio 17 2022" -B build && cmake --build build --config Debug'
```
Release:
```
cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -G "Visual Studio 17 2022" -B build && cmake --build build --config Release'
```

## Windows

### Vulkan
I do not use Vulkan for this app, 
but to remove to remove all warnings during compilation,
I installed it to silence the warning.
* [Vulkan](https://www.lunarg.com/)

* The version I used:
    * https://www.lunarg.com/vulkan-sdk-and-ecosystem-tools-siggraph-2023/
    * https://vulkan.lunarg.com/sdk/home#windows

### MinGW
If you use MinGW will get this:
* Warning: Cannot find any version of the dxcompiler.dll and dxil.dll.
Recommend using MSVC

## Notes and next steps

    Function extraction uses a heuristic regex and brace matching that works well on typical C/C++ method shapes.
      For advanced parsing (templates, lambdas, macros), expand the regex or integrate a lightweight parser.


    The diff algorithm is LCS‑based with a reorder heuristic. It colorizes panes and writes plain text diff into the Comparison pane.
      Saved comparison output uses normal +/‑ markers; reorder is marked with ~.

    Undo/redo: QTextEdit keeps an undo stack; Qt defaults to sufficiently large history.
      You can adjust via document()->setMaximumBlockCount or use a QUndoStack for finer control if needed.

    Settings remember overwrite warning and paths.
      Temp/Projects tabs are scaffolds to support the workflow you described—copying project files into temp, editing, saving,
      and clearing—without touching the original project.

    Menu “Compare (Switch Tab)” moves focus to Compare; toolbar has icons compiled via qrc, so they display correctly on all platforms.

    Testing original/new uses your selected CMake path and builds in a temp build folder inside the temp path.
      You can extend with capture of compiler errors into the Comparison pane for analysis.

If you want me to flesh out the Projects data table with add/save/delete/load stored in QSettings,
or to implement full CMake parsing to preserve complex folder layouts and resource copies, tell me your exact project layout assumptions
and I’ll extend the scaffolding with drop‑in code.


## Diff-Check-AI Project Structure


```
/
├── 📂 icons
│   ├── 📄 app.svg
│   ├── 📄 open.svg
│   ├── 📄 save.svg
│   ├── 📄 compare.svg
│   ├── 📄 about.svg
│   └── 📄 help.svg
├── 📂 translations
│   ├── 📄 Diff-Check-AI\_en.ts
│   ├── 📄 Diff-Check-AI\_es.ts
│   ├── 📄 Diff-Check-AI\_en.qm   # generated
│   └── 📄 Diff-Check-AI\_es.qm   # generated
├── 📂 src
│   ├── 📄 main.cpp
│   ├── 📄 MainWindow.h
│   └── 📄 MainWindow.cpp
│   ├── 📄 CompareEngine.h
│   └── 📄 CompareEngine.cpp
├── 📄 Diff-Check-AI.qrc
└── 📄 CMakeLists.txt

```
