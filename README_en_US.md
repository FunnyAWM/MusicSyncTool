# Music Sync Tool

[中文版本](README.md)
It can automatically detect music files and lyric files and copy them if needed, and also filter favorite music(settings required).
Build with ![Qt](https://img.shields.io/badge/-Qt-41CD52?style=flat-square&logo=qt&logoColor=FFFFFF)

## 1. Installation

This software includes msi package and unpack-and-run version, you can freely choose what you like.

## 2. Building from source

1. Make sure you have Qt library with MSVC compiler, Visual Studio and Vcpkg installed.
2. Clone this repository by the following command and open the .sln file in Visual Studio:
   
   ```
   git clone https://github.com/FunnyAWM/MusicSyncTool.git
   ```
   
   Or you can import this repository directly through Visual Studio.
3. Install Qt VS Tools extension for Visual Studio and set up folder settings for the project.
4. Install taglib and utfcpp in vcpkg by the following command:
   
   ```
   vcpkg install taglib utfcpp
   ```
5. Generate the exe file in both Debug and Release configuration, fix the errors.
6. If you want to generate after you changed the source code, make sure you enabled Qt Deploy Tool after generation under Release configuration, and generate the files. Qt Deploy Tools will automatically find dependencies and copy dll files to the folder where the exe file was.
   
   ## 3. Customize translation
7. Install Qt Linguist。
8. [Download translation template here](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/template.ts)and open it in Qt Linguist.
9. Generate qm files with Qt Linguist and move it to translations folder of the program after translation finished.
10. [Download language configuration tool here](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/Generator.7z) to regenerate the language configurations. The program will automatically detect existing configuration and fill out the info that already exists.
11. Launch Music Sync Tool and check for language settings.
