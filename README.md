​# Diff-Check-AI
When working with AI to write code, expect it to trash your code.
It will drop functions, make up code for a place holder,
and not understand what you are trying to do.
You need to check your files against what AI gives you,
and determine if you want to use it.
This application does a different type of Diff Check for AI.
Instead of doing a file Diff, it does a function Diff,
because most of the time you were only working on one function.
This application will keep track of all the files you altered,
so you can save them back to your project when complete.
It has features to create and edit AI prompts,
so you can keep track of what you asked for,
to compare what you got, and refine your AI prompt to get better results.
I wrote this application to help me deal with AI.


# Table of Contents

- [Diff-Check-AI](https://github.com/AM-Tower/Diff-Check-AI#diff-check-ai)
  - [Status](https://github.com/AM-Tower/Diff-Check-AI#status)
  - [Goals](https://github.com/AM-Tower/Diff-Check-AI#goals)
  - [How-to-use](https://github.com/AM-Tower/Diff-Check-AI#how-to-use)
  - [Build](https://github.com/AM-Tower/Diff-Check-AI#build)
  - [Windows](https://github.com/AM-Tower/Diff-Check-AI#windows)
  - [Notes-and-next-steps](https://github.com/AM-Tower/Diff-Check-AI#notes-and-next-steps)
  - [Diff-Check-AI-Project-Structure](https://github.com/AM-Tower/Diff-Check-AI#diff-check-ai-project-structure)
  - [AI-Instructions](https://github.com/AM-Tower/Diff-Check-AI#ai-instructions)
  - [End-of-README](https://github.com/AM-Tower/Diff-Check-AI#end-of-readme)

## Status
* Not completed, but usable as is, but has bugs, needs a better and faster parser.

What works:
* Compare.
* Compile.
* Settings.
* Backups.

## Goals
* Create a parser class, so I can improve the Diff.
* Add tab Sanitize, to paste in code, and error messages,
and Sanitize it by removing personal information,
like paths, and usernames. It will have an editable list of things to clean.
This is used to send AI clean and sanitize text.
* Tab Prompts, will have add, edit, delete, and other functions,
to maintain a list of prompts, including the one you are working on.
I have another tab AI, where you maintain a list of information you want to give AI.
This is a list of formatting rules, styles, and other things AI needs to know to write code the way you want it.
* Project Manager is a tab that keeps paths, and other settings, so you do not have to edit the settings tab every time.
Overall, it will generate reports on what code you worked on, errors you had, features you have added, and AI prompts.
It will have a feature called Time Card, like the old punch cards some employees used to clock in and out of work.
This will be used to generate time reports.

## How-to-use
1. Go to settings and ensure the Temp and Backup folder are correct.
Browse to CMakeLists.txt, and hit load.
This will delete the Temp folder ProjectName_Temp, and copy all the CMake file and folder structure.
This gives you a temp project to work on.
You only need to load when you make changes in your project,
and need to update the temp project.
This gets backed up under your backup folder in Settings.
2. Click on Open, and locate the file you want to work on in the Temp folder.
Paste or open the file for the new code you want to compare.
Click on the menu Compare.
3. Highlight code you want to transfer from the new side,
and move it or copy it to the original file.
4. Click on Compile, and it will tell you errors, and warnings.
5. Save all changes back to the original project.
6. Test project to ensure all the changes were successful.

If you are working on this application, use this command to run a script
that will make text files combining all the source files into one file.
This is great for history, but you should use git for code history.
This is great for working with AI that does not allow the type of file you want to upload.

Linux
 ```
cd "/mnt/c/Users/$USER/WorkSpace/DiffCheckAI/Qt" \&\& chmod +x src2txt.sh \&\&  dos2unix src2txt.sh \&\&  shellcheck src2txt.sh \&\& ./src2txt.sh --project "DiffCheckAI"
```
Windows
```
Set-Location "C:\Users\$USER\WorkSpace\DiffCheckAI\Qt"; .\src2txt.cmd "DiffCheckAI"
```

Translations update:
```
lupdate.exe ./src -ts translations/DiffCheckAI\_en.ts translations/DiffCheckAI\_es.ts
lrelease.exe translations/DiffCheckAI\_en.ts translations/DiffCheckAI\_es.ts
```

## Build:
Windows:

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
Recommend using MSVC.

## Notes-and-next-steps

    Function extraction uses a heuristic regex and brace matching that works well on typical C/C++ method shapes.
      For advanced parsing (templates, lambdas, macros), expand the regex or integrate a lightweight parser.


    The diff algorithm is LCS‑based with a reorder heuristic. It colorizes panes and writes plain text diff into the Comparison pane.
      Saved comparison output uses normal +/‑ markers; reorder is marked with ~.

    Undo/redo: QTextEdit keeps an undo stack; Qt defaults to sufficiently large history.
      You can adjust via document()->setMaximumBlockCount or use a QUndoStack for finer control if needed.

    Settings remember overwrite warnings and paths.
      Temp/Projects tabs are scaffolds to support the workflow you described—copying project files into temp, editing, saving,
      and clearing—without touching the original project.

    Menu “Compare (Switch Tab)” moves focus to Compare; the toolbar has icons compiled via qrc, so they display correctly on all platforms.

    Testing original/new uses your selected CMake path and builds in a temp build folder inside the temp path.
      You can extend with capture of compiler errors into the Comparison pane for analysis.


## Diff-Check-AI-Project-Structure


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
│   ├── 📄 DiffCheckAI\_en.ts
│   ├── 📄 DiffCheckAI\_es.ts
│   ├── 📄 DiffCheckAI\_en.qm   # generated
│   └── 📄 DiffCheckAI\_es.qm   # generated
├── 📂 src
│   ├── 📄 main.cpp
│   ├── 📄 MainWindow.h
│   └── 📄 MainWindow.cpp
│   ├── 📄 CompareEngine.h
│   └── 📄 CompareEngine.cpp
├── 📄 DiffCheckAI.qrc
└── 📄 CMakeLists.txt

```
## AI-Instructions
AI Instructions:
1. Always use C‑style braces (opening brace on a new line), 
and not JavaScript Style.
Example:
```
void myFunction()
{
    if (condition)
    {
        // code
    }
    else
    {
        // code
    }
}
```

2. Every .cpp and .h file and function must begin with a Doxygen‑style comment block.
Keep each line header with * = 66 characters total to end of line.
* @version 0.6 [Increment] means to increment the number removing instruction [Increment]
same with: 
* @date 2025-11-07 [Todays date], remove [Todays date].
Ensure you are using todays date.
Ensure [AI] gets removed, it is an instruction to ensure your AI Name is in the list, 
do not duplicate.
Like Copilot, ChatGPT, Gemini, Grok, Claude, and so on.
Only alter this line to add yourself, and not add the line above, 
because this should show only the AI that worked on this code.
Ensure to use the correct comment statements depending on file type: cmake uses #
File Example:
```
/****************************************************************
 * @file MainWindow.cpp
 * @brief Implements the main application window.
 *
 * @author Jeffrey Scott Flesher with the help of AI: [AI]
 * @version 0.6 [Increment]
 * @date    2025-11-01 [Todays date]
 * @section License Unlicensed, MIT, or any.
 * @section DESCRIPTION
 * This file contains...
 ***************************************************************/
 ```
Function Example:
```
/****************************************************************
 * @brief Implements the main application window.
 * @param folderPath The absolute path of the directory to scan.
 * @param outputFilePath Absolute path of output file.
 * @return true if the file was successfully, false otherwise.
 ***************************************************************/
 ```
Class Example:
```
/****************************************************************
 * @class MainWindow
 * @brief Implements the main application window.
 ***************************************************************/
 ```
Inside of Class 
public:
signals:
private slots:
private:
all of them
Examples:
public:
```
/****************************************************************
 * @brief Sets command globally.
 * @param Gets version From Settings.
 ***************************************************************/
static void setCommand(const QString &versionFromSettings);
signals:
    /****************************************************************
    * @brief Emitted when output is available.
    * @param output The output text.
    * @param isError true if error output, false for standard output.
    ***************************************************************/
    void outputReceived(const QString &output, bool isError);
```

Define all private variables at bottom of file with comments.
If comments exist, and are adequate, you do not need to alter them.

4. Ensure you do not alter any code or comments you are not working on, 
unless it is to improve them. 
Ensure you do not drop functions, or functionality.
Show all code and function changes in full code, for a drop in replacement.
Show each file one at time, asking if I am ready for next.

5. Indent all functions inside a namespace, but do not add one unless needed,
I do not like to use namespaces unless required:
Example
```
namespace MyNameSpace
{
    void function()
    {
        ...
    }
}
```

6. Every file must end with a marker in this exact format below.
Keep each line header with * = 66 characters total to end of line, code can go over.
Try not to wrap code lines, but keep comments wrapped at 66 characters.
This comment tells me we are the end, I got the full file.
Replace MainWindow.cpp with the actual file name.
Example:
```
/************** End of MainWindow.cpp **************************/
```

7. If Bash make shellcheck compatible. Qt C++ Clazy warning free.
Use index based loops to avoid Clazy warning c++11 range-loop might detach Qt container.

8. Must work on Windows, Linux, and Mac.

9. Always refer to uploads of code, ensuring you do not remove functionality.
If you do not have uploaded code ask.


## End-of-README
