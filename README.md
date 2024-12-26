# 音乐同步工具
[English Version](README_en_US.md)
可以实现对音乐和歌词文件的识别和拷贝，可以识别歌词标签并处理，可以筛选喜欢的音乐（需要设置）。
使用[Qt](https://img.shields.io/badge/-Qt-41CD52?style=flat-square&logo=qt&logoColor=FFFFFF)构建
## 1. 安装
本软件包含msi安装包和绿色版，绿色版可直接解压使用，msi安装包可以安装到指定位置并在桌面生成快捷方式，推荐使用绿色版。
## 2. 从源码构建
1. 确保你正确安装了Qt库、Visual Studio和Vcpkg。
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
## 3. 自定义语言文件
1. 安装Qt Linguist。
2. [从此处下载](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/template.ts)翻译模板并在Qt Linguist中打开。
3. 完成自定义语言文件翻译后，在Qt Linguist中生成qm文件，并移至程序的translations目录下。
4. [从此处下载](https://github.com/FunnyAWM/MusicSyncTool_files/raw/refs/heads/master/Generator.7z)语言配置生成工具重新生成语言配置文件语言。配置生成工具会智能检测当前配置并补全已有配置信息。
5. 启动音乐同步工具并检查语言设置。
