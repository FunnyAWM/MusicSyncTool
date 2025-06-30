# MusicSyncTool Linux 开发指南

## 概述

本文档介绍如何在Linux环境下编译、测试和开发MusicSyncTool项目。该项目是一个基于Qt6的C++音乐同步工具，具有完整的单元测试覆盖。

## 系统要求

### 必需依赖

- **操作系统**: Linux（推荐Ubuntu 20.04+, openSUSE, Fedora等）
- **编译器**: GCC 7+ 或 Clang 6+（支持C++17）
- **CMake**: 3.16+
- **Qt6**: 6.2+（需要Widgets, Sql, Multimedia, Concurrent模块）
- **TagLib**: 1.11+
- **pkg-config**: 用于依赖检测

### 可选依赖（用于测试和代码质量）

- **CTest**: CMake测试框架（CMake自带）
- **lcov/gcov**: 代码覆盖率分析
- **cppcheck**: 静态代码分析
- **clang-tidy**: 代码规范检查

## 依赖安装

### Ubuntu/Debian 系统

```bash
# 更新包管理器
sudo apt update

# 安装基础构建工具
sudo apt install -y build-essential cmake pkg-config git

# 安装Qt6开发包
sudo apt install -y qt6-base-dev qt6-multimedia-dev qt6-tools-dev qt6-l10n-tools

# 安装TagLib
sudo apt install -y libtag1-dev

# 安装测试和分析工具（可选）
sudo apt install -y lcov gcovr cppcheck clang-tidy
```

### openSUSE 系统

```bash
# 安装基础构建工具
sudo zypper install -y gcc-c++ cmake pkg-config git

# 安装Qt6开发包
sudo zypper install -y qt6-base-devel qt6-multimedia-devel qt6-tools-devel

# 安装TagLib
sudo zypper install -y libtag-devel

# 安装测试工具（可选）
sudo zypper install -y lcov cppcheck
```

### Fedora 系统

```bash
# 安装基础构建工具
sudo dnf install -y gcc-c++ cmake pkg-config git

# 安装Qt6开发包
sudo dnf install -y qt6-qtbase-devel qt6-qtmultimedia-devel qt6-qttools-devel

# 安装TagLib
sudo dnf install -y taglib-devel

# 安装测试工具（可选）
sudo dnf install -y lcov cppcheck
```

### 自定义Qt安装

如果使用Qt官方安装器或自定义路径安装Qt，需要修改 `CMakeLists.txt` 中的路径：

```cmake
# 将此行改为您的Qt安装路径
set(CMAKE_PREFIX_PATH "/path/to/your/qt/installation")
```

## 快速开始

### 1. 克隆项目

```bash
git clone <repository-url>
cd MusicSyncTool
```

### 2. 使用自动化脚本

项目提供了便捷的构建脚本：

```bash
# 赋予脚本执行权限
chmod +x build_and_test.sh

# 构建主项目
./build_and_test.sh

# 构建并运行单元测试
./build_and_test.sh test

# 清理构建目录
./build_and_test.sh clean

# 显示帮助信息
./build_and_test.sh help
```

### 3. 手动构建

#### 构建主项目

```bash
mkdir -p build-linux
cd build-linux
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### 构建和运行单元测试

```bash
mkdir -p build-test-linux
cd build-test-linux
cmake ../UnitTest -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

## 单元测试详细说明

### 测试结构

```
UnitTest/
├── CMakeLists.txt          # CMake配置文件
├── main.cpp                # 测试入口
├── Test*.h                 # 测试头文件
├── Test*.cpp               # 测试实现文件
├── testdata/               # 测试数据目录
├── run_tests.sh            # Linux测试脚本
├── run_tests.bat           # Windows测试脚本
└── README.md               # 测试文档
```

### 测试覆盖范围

1. **MSTFileManager**: 文件管理功能测试
   - 文件操作（创建、删除、移动）
   - 路径处理和验证
   - 权限检查
   - 异常处理

2. **MSTDataSource**: 数据源管理测试
   - 数据库连接和操作
   - 数据查询和更新
   - 事务处理
   - 错误恢复

3. **QueryItem**: 查询项测试
   - 构造和初始化
   - 数据访问和修改
   - 序列化和反序列化
   - 比较操作

4. **LyricIgnoreRule**: 歌词规则测试
   - 规则创建和验证
   - 字符串转换
   - 规则匹配逻辑
   - 边界条件

5. **Logger**: 日志系统测试
   - 不同级别日志记录
   - 线程安全性
   - 性能测试
   - 并发写入

### 运行特定测试

```bash
cd build-test-linux

# 运行所有测试
ctest

# 运行特定测试
ctest -R TestMSTFileManager

# 详细输出
ctest --output-on-failure --verbose

# 并行运行测试
ctest -j$(nproc)
```

### 生成测试报告

```bash
# XML格式报告
ctest --output-junit testresults.xml

# 在构建目录查看详细结果
cat Testing/Temporary/LastTest.log
```

## 代码覆盖率分析

### 生成覆盖率报告

```bash
mkdir -p build-coverage
cd build-coverage

# 配置构建（启用覆盖率）
cmake ../UnitTest \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
    -DCMAKE_C_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage"

# 构建和运行测试
make -j$(nproc)
ctest

# 生成覆盖率报告
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/UnitTest/*' '*/Qt6/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage-html

# 在浏览器中查看报告
xdg-open coverage-html/index.html
```

## 代码质量检查

### 静态分析

```bash
# 使用cppcheck进行静态分析
cppcheck --enable=all --xml --xml-version=2 src/ 2> cppcheck-report.xml

# 查看结果
cat cppcheck-report.xml
```

### 代码规范检查

```bash
# 使用clang-tidy检查代码规范
find src/ -name "*.cpp" -o -name "*.h" | xargs clang-tidy -p build-linux/
```

## 持续集成

### GitHub Actions

项目包含完整的CI/CD配置（`.github/workflows/ci.yml`），支持：

- **多平台构建**: Linux, macOS, Windows
- **多Qt版本测试**: Qt 6.5, 6.7
- **自动化测试**: 单元测试自动运行
- **代码质量**: 静态分析和覆盖率报告
- **构建产物**: 自动打包和发布

### 本地CI验证

```bash
# 模拟CI环境进行完整测试
./build_and_test.sh clean
./build_and_test.sh test
```

## 开发工作流

### 1. 开发新功能

```bash
# 创建功能分支
git checkout -b feature/new-feature

# 开发代码
# ...

# 运行测试确保质量
./build_and_test.sh test

# 提交更改
git add .
git commit -m "Add new feature"
git push origin feature/new-feature
```

### 2. 添加新的单元测试

```bash
# 编辑测试文件
vim UnitTest/TestNewFeature.cpp

# 更新CMakeLists.txt添加新测试
vim UnitTest/CMakeLists.txt

# 运行新测试
./build_and_test.sh test
```

### 3. 性能调优

```bash
# 生成性能分析版本
mkdir -p build-profile
cd build-profile
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)

# 使用profiling工具
perf record ./MusicSyncTool
perf report
```

## 故障排除

### 常见问题

1. **Qt找不到**
   ```bash
   # 确保Qt在PATH中
   export PATH=/path/to/qt/bin:$PATH
   export CMAKE_PREFIX_PATH=/path/to/qt
   ```

2. **TagLib缺失**
   ```bash
   # 检查pkg-config
   pkg-config --libs taglib
   
   # 手动指定路径
   cmake .. -DTagLib_ROOT=/path/to/taglib
   ```

3. **测试失败**
   ```bash
   # 查看详细日志
   ctest --output-on-failure --verbose
   
   # 运行特定失败的测试
   ctest -R FailedTestName --verbose
   ```

4. **权限问题**
   ```bash
   # 确保有足够权限
   chmod +x build_and_test.sh
   chmod -R 755 UnitTest/
   ```

### 日志和调试

```bash
# 启用详细CMake日志
cmake .. --debug-output

# 启用Qt日志
export QT_LOGGING_RULES="*.debug=true"

# 运行gdb调试
gdb ./UnitTestRunner
```

## 贡献指南

1. **代码风格**: 遵循项目既有的编码规范
2. **测试要求**: 新功能必须包含相应的单元测试
3. **文档更新**: 重要功能需要更新相关文档
4. **CI通过**: 所有测试必须通过才能合并

## 性能基准

在典型的Linux开发环境（Intel i7, 16GB RAM）下：

- **完整构建时间**: ~2-5分钟
- **单元测试运行时间**: ~10-30秒
- **覆盖率生成时间**: ~1-2分钟

## 联系和支持

- **问题报告**: 通过GitHub Issues
- **功能请求**: 通过GitHub Discussions
- **开发讨论**: 参考项目README中的联系方式
