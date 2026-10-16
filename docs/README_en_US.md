# Music Sync Tool

[中文版本](README.md)

It can automatically detect music files and lyric files and copy them if needed, and also filter favorite music(settings required).
Build with [Qt](https://img.shields.io/badge/-Qt-41CD52?style=flat-square&logo=qt&logoColor=FFFFFF)

## 1. Features

- Detects and batch-copies music and lyric files; free space on the target disk is checked first, and failed files are rolled back and collected in the error list;
- Lyric tag handling: customisable lyric ignore rules, matching on title/artist/album with includes/excludes mode;
- Favourite filtering: mark favourite music with a custom tag and list it separately;
- Duplicate detection: fuzzy comparison on title, artist and album to list duplicated files;
- Recursive scan: optionally scan the sub-directories of the selected folder;
- Themes: light/dark mode combined with blue, green, purple or dark accent colours, applied immediately after saving;
- Localization: built-in Simplified Chinese, English and Classical Chinese, with support for custom language files (see §4);
- Logging: optionally write the run log to `log/lastRun.log` under the program directory.

## 2. Installation

This software includes msi package and unpack-and-run version, you can freely choose what you like.

## 3. Building from source

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
   
   Linux and macOS can be built with CMake (prepare Qt6, TagLib and utfcpp yourself); on Windows use the Visual Studio project above.
   
## 4. Customize translation

1. Install Qt Linguist。
2. [Download translation template here](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/template.ts)and open it in Qt Linguist.
3. Generate qm files with Qt Linguist and move it to translations folder of the program after translation finished.
4. [Download language configuration tool here](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/Generator.7z) to regenerate the language configurations. The program will automatically detect existing configuration and fill out the info that already exists.
5. Launch Music Sync Tool and check for language settings.

## 5. Settings and runtime files

| File | Location | Description |
| --- | --- | --- |
| `settings.json` | program directory | User settings: language, sort field and direction, ignore lyrics, recursive scan, file logging, theme (mode + accent colour) and ignore rules |
| `musicInfo.db` | every scanned music directory | Metadata cache (SQLite); missing columns of an old database are added on startup |
| `log/lastRun.log` | program directory/log | Run log written when "log to file" is enabled |

See [classes/en_US/MusicSyncTool.md](classes/en_US/MusicSyncTool.md) for the architecture and class reference.
