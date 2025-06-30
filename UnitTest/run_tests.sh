#!/bin/bash

# MusicSyncTool 单元测试运行脚本
# 基于主项目配置方式，确保一致性

set -e  # 遇到错误时退出

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$SCRIPT_DIR/build"

# 颜色输出定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE} MusicSyncTool 单元测试${NC}"
echo -e "${BLUE}================================================${NC}"

# 检查系统支持
check_system_support() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        print_status "检测到Linux系统"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        print_status "检测到macOS系统"
    else
        print_error "不支持的系统类型: $OSTYPE"
        print_error "此脚本仅支持Linux和macOS系统"
        exit 1
    fi
}

# 检查依赖工具
check_dependencies() {
    print_status "检查构建依赖..."
    
    # 检查CMake版本
    if ! command -v cmake &> /dev/null; then
        print_error "CMake 未安装。请安装 CMake 3.16 或更高版本"
        exit 1
    fi
    
    local cmake_version=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_status "CMake版本: $cmake_version"
    
    # 检查编译器
    if command -v g++ &> /dev/null; then
        local gcc_version=$(g++ --version | head -n1)
        print_status "编译器: $gcc_version"
    elif command -v clang++ &> /dev/null; then
        local clang_version=$(clang++ --version | head -n1)
        print_status "编译器: $clang_version"
    else
        print_error "未找到C++编译器 (g++ 或 clang++)"
        exit 1
    fi
    
    # 检查Qt6
    if command -v qmake6 &> /dev/null; then
        local qt_version=$(qmake6 -query QT_VERSION)
        print_status "Qt版本: $qt_version"
    elif [[ -n "$Qt6_DIR" ]]; then
        print_status "使用环境变量Qt6_DIR: $Qt6_DIR"
    else
        print_warning "未检测到Qt6，将使用系统默认安装或CMake查找"
    fi
    
    # 检查TagLib
    if pkg-config --exists taglib; then
        local taglib_version=$(pkg-config --modversion taglib)
        print_status "TagLib版本: $taglib_version"
    else
        print_warning "未找到TagLib pkg-config，请确保已安装TagLib开发包"
    fi
}

# 清理并创建构建目录
prepare_build_dir() {
    print_status "准备构建目录..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_status "清理了现有构建目录"
    fi
    
    mkdir -p "$BUILD_DIR"
    print_success "构建目录创建完成: $BUILD_DIR"
}

# 配置构建
configure_build() {
    print_status "配置CMake构建..."
    
    cd "$BUILD_DIR"
    
    # 构建配置选项
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=${BUILD_TYPE:-Debug}"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )
    
    # 如果设置了Qt6路径
    if [[ -n "$Qt6_DIR" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=$Qt6_DIR")
        print_status "使用Qt6路径: $Qt6_DIR"
    fi
    
    # 运行CMake配置
    if cmake "${cmake_args[@]}" ..; then
        print_success "CMake配置成功"
    else
        print_error "CMake配置失败"
        exit 1
    fi
}

# 构建测试
build_tests() {
    print_status "构建单元测试..."
    
    cd "$BUILD_DIR"
    
    # 使用所有可用CPU核心进行编译
    local num_cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    print_status "使用 $num_cores 个CPU核心进行编译"
    
    if make -j"$num_cores"; then
        print_success "测试构建完成"
    else
        print_error "测试构建失败"
        exit 1
    fi
}

# 运行测试
run_tests() {
    print_status "运行单元测试..."
    
    cd "$BUILD_DIR"
    
    # 设置Qt平台为离屏模式
    export QT_QPA_PLATFORM=offscreen
    
    echo -e "${BLUE}================================${NC}"
    
    # 首先尝试使用CTest运行
    if command -v ctest &> /dev/null; then
        if ctest --output-on-failure; then
            print_success "CTest: 所有测试通过"
            return 0
        else
            print_warning "CTest: 部分测试失败，尝试直接运行测试程序"
        fi
    fi
    
    # 直接运行测试可执行文件
    if [ -f "./MusicSyncToolTests" ]; then
        print_status "直接运行测试程序..."
        if ./MusicSyncToolTests; then
            print_success "直接测试: 所有测试通过"
            return 0
        else
            print_error "直接测试: 测试失败"
            return 1
        fi
    else
        print_error "测试可执行文件未找到: ./MusicSyncToolTests"
        return 1
    fi
}

# 显示测试结果
show_results() {
    local test_result=$1
    
    echo -e "${BLUE}================================${NC}"
    
    if [ $test_result -eq 0 ]; then
        print_success "🎉 所有测试通过！"
        print_status "测试报告已生成在: $BUILD_DIR"
        print_status "编译数据库: $BUILD_DIR/compile_commands.json"
    else
        print_error "❌ 测试失败，退出码: $test_result"
        print_status "请检查上述错误信息并修复相关问题"
    fi
}

# 主函数
main() {
    # 检查是否在正确的目录
    if [ ! -f "$SCRIPT_DIR/CMakeLists.txt" ]; then
        print_error "错误：请确保在UnitTest目录下运行此脚本"
        print_error "当前目录: $SCRIPT_DIR"
        exit 1
    fi
    
    # 设置默认构建类型
    BUILD_TYPE=${BUILD_TYPE:-Debug}
    
    print_status "开始MusicSyncTool单元测试"
    print_status "构建类型: $BUILD_TYPE"
    print_status "工作目录: $SCRIPT_DIR"
    
    # 执行构建和测试流程
    check_system_support
    check_dependencies
    prepare_build_dir
    configure_build
    build_tests
    
    # 运行测试并获取结果
    local test_result=0
    if ! run_tests; then
        test_result=1
    fi
    
    show_results $test_result
    exit $test_result
}

# 脚本入口点
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
