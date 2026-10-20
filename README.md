# 音乐同步工具

[![MusicSyncTool CI](https://github.com/FunnyAWM/MusicSyncTool/actions/workflows/ci.yml/badge.svg)](https://github.com/FunnyAWM/MusicSyncTool/actions/workflows/ci.yml)

[English Version](README_en_US.md)

可以实现对音乐和歌词文件的识别和拷贝，可以识别歌词标签并处理，可以筛选喜欢的音乐（需要设置）。
使用![Qt](https://img.shields.io/badge/-Qt-41CD52?style=flat-square&logo=qt&logoColor=FFFFFF)构建

## 1. 功能

- 音乐与歌词文件的识别和批量拷贝，拷贝前检查目标磁盘空间，失败的文件会回滚并加入错误列表；
- 拷贝可中途取消：点击进度条下方的“取消”后，当前歌曲与其歌词拷贝完成即停止，已完成文件保留；
- 歌词标签处理：自定义歌词忽略规则，匹配字段可选名称/艺术家/专辑，匹配方式可选包含/排除；
- 收藏筛选：按自定义标签标记喜欢的音乐并单独列出；
- 重复项检测：按标题、艺术家、专辑模糊比较，列出重复文件；
- 递归扫描：可选择是否扫描所选目录下的子目录；
- 主题：明亮/暗色模式搭配蓝、绿、紫、深色强调色，保存设置后即时生效；
- 多语言：内置简体中文、English、文言，支持自定义语言文件（见 §4）；
- 日志：可选择将运行日志写入程序目录下的 `log/lastRun.log`。

## 2. 安装

本软件包含msi安装包和绿色版，绿色版可直接解压使用，msi安装包可以安装到指定位置并在桌面生成快捷方式，推荐使用绿色版。

## 3. 从源码构建

1. 确保你正确安装了Qt库、Visual Studio和Vcpkg。Windows 构建基线为 **Qt 6.8 LTS（`6.8.3_msvc2022_64`）**，工程内已配置该版本；使用其他 6.8.x 版本时需同步修改工程属性或 Qt VS Tools 中的 Qt 版本。
2. 拉取MusicSyncTool代码到本地，使用如下命令并在VS中打开sln文件：
   
   ```
   git clone https://github.com/FunnyAWM/MusicSyncTool.git
   ```
   
   或从VS中通过VCS导入代码。
3. 在VS中安装Qt VS Tools扩展，并调整Qt文件夹设置。
4. 通过Vcpkg安装utfcpp、taglib包，运行如下命令：
   
   ```
   vcpkg install taglib utfcpp
   ```
5. 在Debug和Release模式下分别进行一次源码编译，排除问题。
6. 如果需要更改源码，在更改源码后再次进行Release编译，Qt工具会自动检查依赖库文件并补全。
   
   Linux和macOS可通过CMake构建（需自行准备Qt6、TagLib与utfcpp），Windows下请使用上述VS工程。
   

## 4. 自定义语言文件

1. 安装Qt Linguist。
2. [从此处下载](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/template.ts)翻译模板并在Qt Linguist中打开。
3. 完成自定义语言文件翻译后，在Qt Linguist中生成qm文件，并移至程序的translations目录下。
4. [从此处下载](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/Generator.7z)语言配置生成工具重新生成语言配置文件语言。配置生成工具会智能检测当前配置并补全已有配置信息。
5. 启动音乐同步工具并检查语言设置。

## 5. 设置与运行时文件

| 文件 | 位置 | 说明 |
| --- | --- | --- |
| `settings.json` | 程序目录 | 用户设置：语言、排序字段与升降序、忽略歌词、递归扫描、日志落盘、主题（明暗 + 强调色）与忽略规则 |
| `musicInfo.db` | 每个被扫描的音乐目录 | 音乐元数据缓存（SQLite），旧库会在启动时自动补齐缺失列 |
| `log/lastRun.log` | 程序目录/log | 开启“日志写入文件”后的运行日志 |

类与分层说明见 [docs/classes/zh_CN/MusicSyncTool.md](docs/classes/zh_CN/MusicSyncTool.md)。
