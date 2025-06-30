#!/bin/bash

# MusicSyncTool Linux 构建和测试脚本
# 使用方法:
# ./build_and_test.sh         # 构建主项目
# ./build_and_test.sh test    # 构建并运行测试
# ./build_and_test.sh clean   # 清理构建目录

set -e  # 遇到错误时退出

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-linux"
TEST_BUILD_DIR="$PROJECT_ROOT/build-test-linux"

# 颜色输出
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

# 检查系统和依赖
check_dependencies() {
    print_status "检查构建依赖..."
    
    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake 未安装。请安装 CMake 3.16 或更高版本"
        exit 1
    fi
    
    # 检查Qt6
    if ! pkg-config --exists Qt6Widgets; then
        print_warning "未找到 Qt6 pkg-config。请确保已安装 Qt6 开发包"
        print_warning "如使用自定义 Qt 路径，请修改 CMakeLists.txt 中的 CMAKE_PREFIX_PATH"
    fi
    
    # 检查TagLib
    if ! pkg-config --exists taglib; then
        print_warning "未找到 TagLib pkg-config。请确保已安装 TagLib 开发包"
    fi
    
    print_success "依赖检查完成"
}

# 构建主项目
build_main() {
    print_status "构建主项目..."
    
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    
    print_success "主项目构建完成"
}

# 构建并运行测试
build_and_run_tests() {
    print_status "构建并运行单元测试..."
    
    mkdir -p "$TEST_BUILD_DIR"
    cd "$TEST_BUILD_DIR"
    
    # 配置测试构建 - 与主项目保持一致的配置方式
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=Debug"
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    )
    
    # 如果设置了Qt6路径，传递给测试构建
    if [[ -n "$Qt6_DIR" ]]; then
        cmake_args+=("-DCMAKE_PREFIX_PATH=$Qt6_DIR")
        print_status "使用Qt6路径: $Qt6_DIR"
    fi
    
    # 可选功能
    if [[ "$ENABLE_COVERAGE" == "ON" ]]; then
        cmake_args+=("-DENABLE_COVERAGE=ON")
        print_status "启用代码覆盖率报告"
    fi
    
    if [[ "$ENABLE_SANITIZER" == "ON" ]]; then
        cmake_args+=("-DENABLE_SANITIZER=ON")
        print_status "启用地址清理器"
    fi
    
    if cmake "${cmake_args[@]}" ../UnitTest; then
        print_success "测试CMake配置成功"
    else
        print_error "测试CMake配置失败"
        return 1
    fi
    
    # 构建测试
    if make -j$(nproc); then
        print_success "测试构建完成"
    else
        print_error "测试构建失败"
        return 1
    fi
    
    # 运行测试
    print_status "运行单元测试..."
    export QT_QPA_PLATFORM=offscreen
    
    if ctest --output-on-failure; then
        print_success "所有测试通过!"
        
        # 如果启用了覆盖率，生成报告
        if [[ "$ENABLE_COVERAGE" == "ON" ]] && command -v lcov &> /dev/null; then
            print_status "生成覆盖率报告..."
            lcov --capture --directory . --output-file coverage.info
            lcov --remove coverage.info '/usr/*' '*/Qt6/*' '*/build/*' --output-file coverage_filtered.info
            
            if command -v genhtml &> /dev/null; then
                genhtml coverage_filtered.info --output-directory coverage_html
                print_success "覆盖率HTML报告: $TEST_BUILD_DIR/coverage_html/"
            fi
        fi
        
        return 0
    else
        print_error "部分测试失败"
        return 1
    fi
}

# 清理构建目录
clean_build() {
    print_status "清理构建目录..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "清理主项目构建目录: $BUILD_DIR"
    fi
    
    if [ -d "$TEST_BUILD_DIR" ]; then
        rm -rf "$TEST_BUILD_DIR"
        print_success "清理测试构建目录: $TEST_BUILD_DIR"
    fi
    
    # 清理其他构建目录
    if [ -d "$PROJECT_ROOT/build" ]; then
        rm -rf "$PROJECT_ROOT/build"
        print_success "清理构建目录: $PROJECT_ROOT/build"
    fi
    
    if [ -d "$PROJECT_ROOT/cmake-build-debug" ]; then
        rm -rf "$PROJECT_ROOT/cmake-build-debug"
        print_success "清理构建目录: $PROJECT_ROOT/cmake-build-debug"
    fi
}

# 显示帮助信息
show_help() {
    echo "MusicSyncTool Linux 构建和测试脚本"
    echo ""
    echo "使用方法:"
    echo "  $0                构建主项目"
    echo "  $0 test          构建并运行单元测试"
    echo "  $0 coverage      构建测试并生成覆盖率报告"
    echo "  $0 sanitizer     使用地址清理器构建测试"
    echo "  $0 clean         清理所有构建目录"
    echo "  $0 help          显示此帮助信息"
    echo ""
    echo "环境变量:"
    echo "  Qt6_DIR          Qt6安装目录路径"
    echo "  BUILD_TYPE       构建类型 (Debug/Release，默认Release)"
    echo ""
    echo "环境要求:"
    echo "  - CMake 3.16+"
    echo "  - Qt6 (Widgets, Sql, Multimedia, Concurrent, Test)"
    echo "  - TagLib"
    echo "  - 支持 C++17 的编译器"
    echo ""
    echo "注意:"
    echo "  - 如果使用自定义 Qt 安装路径，请设置 Qt6_DIR 环境变量"
    echo "  - 确保所有依赖库都已正确安装"
    echo "  - 覆盖率报告需要安装 lcov 和 genhtml"
}

# 主逻辑
main() {
    cd "$PROJECT_ROOT"
    
    # 设置默认值
    ENABLE_COVERAGE=OFF
    ENABLE_SANITIZER=OFF
    
    case "${1:-build}" in
        "test")
            check_dependencies
            build_and_run_tests
            ;;
        "coverage")
            ENABLE_COVERAGE=ON
            check_dependencies
            build_and_run_tests
            ;;
        "sanitizer")
            ENABLE_SANITIZER=ON
            check_dependencies
            build_and_run_tests
            ;;
        "clean")
            clean_build
            ;;
        "help"|"-h"|"--help")
            show_help
            ;;
        "build"|"")
            check_dependencies
            build_main
            ;;
        *)
            print_error "未知命令: $1"
            show_help
            exit 1
            ;;
    esac
}

# 如果脚本被直接执行
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
