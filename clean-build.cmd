@echo off
echo Environment cleaning will delete build folder and CMakeLists.txt.user:
REM Delete all contents of build folder
IF EXIST build (
    rmdir /s /q build
    mkdir build
)

REM Delete QtCreator user file
IF EXIST .qtcreator\CMakeLists.txt.user (
    del /q .qtcreator\CMakeLists.txt.user
)

REM Delete QtCreator user file in root folder
IF EXIST CMakeLists.txt.user (
    del /q CMakeLists.txt.user
)

echo Environment cleaned. Ready for fresh build.
pause