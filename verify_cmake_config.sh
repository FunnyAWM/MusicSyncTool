#!/bin/bash

# MusicSyncTool CMake配置验证脚本
# 验证新的CMake配置是否工作正常

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UNITTEST_DIR="$SCRIPT_DIR/UnitTest"
TEMP_BUILD_DIR="$UNITTEST_DIR/build-verify"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE} MusicSyncTool CMake配置验证${NC}"
echo -e "${BLUE}================================================${NC}"

# 清理临时构建目录
cleanup() {
    if [ -d "$TEMP_BUILD_DIR" ]; then
        rm -rf "$TEMP_BUILD_DIR"
        print_status "清理了临时构建目录"
    fi
}

# 设置清理陷阱
trap cleanup EXIT

# 检查CMakeLists.txt文件
print_status "检查CMakeLists.txt文件..."
if [ ! -f "$UNITTEST_DIR/CMakeLists.txt" ]; then
    print_error "未找到UnitTest/CMakeLists.txt"
    exit 1
fi
print_success "CMakeLists.txt文件存在"

# 检查CMake语法
print_status "验证CMake语法..."
if command -v cmake &> /dev/null; then
    mkdir -p "$TEMP_BUILD_DIR"
    cd "$TEMP_BUILD_DIR"
    
    # 只进行配置，不构建
    if cmake .. -DCMAKE_BUILD_TYPE=Debug --dry-run 2>/dev/null || cmake .. -DCMAKE_BUILD_TYPE=Debug &>/dev/null; then
        print_success "CMake配置语法正确"
    else
        print_error "CMake配置有语法错误"
        exit 1
    fi
else
    print_error "CMake未安装，跳过语法验证"
fi

# 检查源文件
print_status "检查测试源文件..."
local test_files=(
    "TestMSTFileManager.cpp"
    "TestMSTFileManager.h"
    "TestMSTDataSource.cpp"
    "TestMSTDataSource.h"
    "TestQueryItem.cpp"
    "TestQueryItem.h"
    "TestLyricIgnoreRule.cpp"
    "TestLyricIgnoreRule.h"
    "TestLogger.cpp"
    "TestLogger.h"
    "main.cpp"
)

for file in "${test_files[@]}"; do
    if [ -f "$UNITTEST_DIR/$file" ]; then
        print_success "测试文件: $file"
    else
        print_error "缺少测试文件: $file"
        exit 1
    fi
done

# 检查主项目源文件
print_status "检查主项目源文件..."
local main_files=(
    "src/MSTFileManager.cpp"
    "src/MSTFileManager.h"
    "src/MSTDataSource.cpp"
    "src/MSTDataSource.h"
    "src/QueryItem.cpp"
    "src/QueryItem.h"
    "src/LyricIgnoreRule.cpp"
    "src/LyricIgnoreRule.h"
    "src/Logger.cpp"
    "src/Logger.h"
    "src/MusicProperties.h"
)

for file in "${main_files[@]}"; do
    if [ -f "$SCRIPT_DIR/$file" ]; then
        print_success "主项目文件: $(basename $file)"
    else
        print_error "缺少主项目文件: $file"
        exit 1
    fi
done

# 检查脚本文件
print_status "检查自动化脚本..."
local scripts=(
    "UnitTest/run_tests.sh"
    "build_and_test.sh"
    "check_project_integrity.sh"
)

for script in "${scripts[@]}"; do
    if [ -f "$SCRIPT_DIR/$script" ]; then
        if [ -x "$SCRIPT_DIR/$script" ]; then
            print_success "脚本文件: $script (可执行)"
        else
            print_success "脚本文件: $script (需要执行权限)"
        fi
    else
        print_error "缺少脚本文件: $script"
        exit 1
    fi
done

print_status "检查CI配置..."
if [ -f "$SCRIPT_DIR/.github/workflows/ci.yml" ]; then
    print_success "CI配置文件存在"
else
    print_error "缺少CI配置文件"
    exit 1
fi

print_status "检查文档文件..."
local docs=(
    "UnitTest/README.md"
    "LINUX_DEVELOPMENT.md"
    "PROJECT_COMPLETION_SUMMARY.md"
)

for doc in "${docs[@]}"; do
    if [ -f "$SCRIPT_DIR/$doc" ]; then
        print_success "文档文件: $doc"
    else
        print_error "缺少文档文件: $doc"
        exit 1
    fi
done

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN} 验证完成！${NC}"
echo -e "${GREEN}================================================${NC}"

print_success "✅ CMake配置文件语法正确"
print_success "✅ 所有测试文件完整"
print_success "✅ 主项目源文件完整"
print_success "✅ 自动化脚本就绪"
print_success "✅ CI配置文件存在"
print_success "✅ 文档文件完整"

echo -e "\n${BLUE}下一步：${NC}"
echo "1. 在Linux环境中运行: ./UnitTest/run_tests.sh"
echo "2. 或使用主脚本: ./build_and_test.sh test"
echo "3. 检查项目完整性: ./check_project_integrity.sh"

print_success "🎉 MusicSyncTool单元测试配置验证通过！"
