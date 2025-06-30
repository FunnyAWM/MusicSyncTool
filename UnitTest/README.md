# Music Sync Tool 单元测试

本目录包含 Music Sync Tool 项目的单元测试代码。

## 系统要求

- Linux 操作系统
- Qt6 开发环境
- CMake 3.16+
- TagLib 库
- GCC 或 Clang 编译器（支持C++20）

## 依赖安装

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install \
    qt6-base-dev \
    qt6-multimedia-dev \
    cmake \
    build-essential \
    libtag1-dev \
    pkg-config
```

### Fedora/CentOS/RHEL
```bash
sudo dnf install \
    qt6-qtbase-devel \
    qt6-qtmultimedia-devel \
    cmake \
    gcc-c++ \
    taglib-devel \
    pkgconfig
```

### Arch Linux
```bash
sudo pacman -S \
    qt6-base \
    qt6-multimedia \
    cmake \
    gcc \
    taglib \
    pkgconf
```

## 构建和运行测试

### 方法一：使用脚本（推荐）
```bash
chmod +x run_tests.sh
./run_tests.sh
```

### 方法二：手动构建
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
export QT_QPA_PLATFORM=offscreen
./MusicSyncToolTests
```

## 测试覆盖范围

### 1. MSTFileManager 测试 (TestMSTFileManager)
- 文件格式支持检查
- 磁盘空间检查
- 文件复制回滚功能
- 存储空间信息获取
- 边界条件和错误处理

### 2. QueryItem 测试 (TestQueryItem)
- 音乐元数据构造和访问
- Getter/Setter 方法
- 相似度比较算法
- 中文字符处理
- 边界值测试

### 3. LyricIgnoreRule 测试 (TestLyricIgnoreRule)
- 规则构造和比较
- 字符串与枚举转换
- 拷贝构造和赋值
- 中文规则名称处理
- 所有规则类型组合

### 4. Logger 测试 (TestLogger)
- 不同级别日志记录
- 特殊字符和长消息处理
- 中文日志消息
- 并发日志记录安全性
- 边界条件测试

### 5. MSTDataSource 测试 (TestMSTDataSource)
- 数据库连接和表初始化
- 音乐数据的CRUD操作
- 分页查询和排序
- 搜索功能
- 收藏和规则命中功能
- 信号发射机制
- 错误处理和边界条件

## 测试策略

### 单元测试原则
- **隔离性**：每个测试用例都是独立的
- **可重复性**：测试结果应该是稳定和可重复的
- **快速执行**：测试应该快速完成
- **清晰断言**：每个测试都有明确的预期结果

### 测试数据管理
- 使用临时目录和文件避免污染系统
- 每个测试前后都进行清理
- 使用模拟数据而不依赖外部文件

### 边界条件测试
- 空值和null输入
- 极大和极小数值
- 特殊字符和Unicode
- 并发访问情况

## 测试输出

测试运行后会显示：
- 每个测试类的执行结果
- 失败测试的详细信息
- 总体通过率
- 执行时间统计

## 故障排除

### 常见问题

1. **Qt6 未找到**
   ```
   错误：未找到Qt6
   ```
   解决：安装Qt6开发包并确保pkg-config能找到Qt6

2. **TagLib 未找到**
   ```
   错误：未找到TagLib
   ```
   解决：安装TagLib开发包

3. **编译错误**
   ```
   C++20 features not supported
   ```
   解决：确保使用支持C++20的编译器版本

4. **运行时图形错误**
   ```
   QApplication: no display available
   ```
   解决：设置环境变量 `export QT_QPA_PLATFORM=offscreen`

### 调试技巧

1. **详细输出模式**
   ```bash
   ./MusicSyncToolTests -v2
   ```

2. **运行特定测试**
   ```bash
   ./MusicSyncToolTests TestMSTFileManager
   ```

3. **生成XML报告**
   ```bash
   ./MusicSyncToolTests -xml -o test_results.xml
   ```

## 持续集成

可以将此测试套件集成到CI/CD流水线中：

```yaml
# 示例GitHub Actions配置
test:
  runs-on: ubuntu-latest
  steps:
    - uses: actions/checkout@v3
    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install qt6-base-dev qt6-multimedia-dev libtag1-dev cmake build-essential
    - name: Run tests
      run: |
        cd UnitTest
        chmod +x run_tests.sh
        ./run_tests.sh
```

## 贡献指南

### 添加新测试
1. 在相应的测试类中添加新的测试方法
2. 遵循现有的命名约定（test + 功能描述）
3. 确保测试的独立性和清理工作
4. 添加适当的文档注释

### 测试最佳实践
- 一个测试方法只测试一个功能点
- 使用描述性的测试方法名
- 添加足够的断言验证结果
- 处理可能的异常情况
- 保持测试代码的简洁和可读性

## 许可证

此测试代码遵循与主项目相同的许可证。
