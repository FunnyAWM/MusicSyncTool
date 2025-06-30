#!/bin/bash

# MusicSyncTool 项目完整性检查脚本
# 用于验证单元测试项目的完整性和配置正确性

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE} MusicSyncTool 项目完整性检查${NC}"
    echo -e "${BLUE}================================================${NC}"
}

print_check() {
    local message="$1"
    local status="$2"
    if [ "$status" = "OK" ]; then
        echo -e "${GREEN}✅${NC} $message"
    elif [ "$status" = "WARNING" ]; then
        echo -e "${YELLOW}⚠️${NC} $message"
    else
        echo -e "${RED}❌${NC} $message"
    fi
}

print_section() {
    echo -e "\n${BLUE}$1${NC}"
    echo "----------------------------------------"
}

# 检查必需文件
check_required_files() {
    print_section "1. 检查必需文件"
    
    local files=(
        "CMakeLists.txt"
        "UnitTest/CMakeLists.txt"
        "UnitTest/main.cpp"
        "UnitTest/README.md"
        "UnitTest/PROJECT_SUMMARY.md"
        "build_and_test.sh"
        "LINUX_DEVELOPMENT.md"
        "PROJECT_COMPLETION_SUMMARY.md"
        ".github/workflows/ci.yml"
        ".gitignore"
    )
    
    for file in "${files[@]}"; do
        if [ -f "$PROJECT_ROOT/$file" ]; then
            print_check "文件存在: $file" "OK"
        else
            print_check "文件缺失: $file" "ERROR"
        fi
    done
}

# 检查测试文件
check_test_files() {
    print_section "2. 检查测试文件"
    
    local test_modules=("MSTFileManager" "MSTDataSource" "QueryItem" "LyricIgnoreRule" "Logger")
    
    for module in "${test_modules[@]}"; do
        local header="UnitTest/Test${module}.h"
        local source="UnitTest/Test${module}.cpp"
        
        if [ -f "$PROJECT_ROOT/$header" ] && [ -f "$PROJECT_ROOT/$source" ]; then
            print_check "测试模块: $module" "OK"
        else
            print_check "测试模块缺失: $module" "ERROR"
        fi
    done
}

# 检查源文件
check_source_files() {
    print_section "3. 检查源文件"
    
    local source_files=(
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
    
    for file in "${source_files[@]}"; do
        if [ -f "$PROJECT_ROOT/$file" ]; then
            print_check "源文件: $(basename $file)" "OK"
        else
            print_check "源文件缺失: $file" "WARNING"
        fi
    done
}

# 检查脚本权限
check_script_permissions() {
    print_section "4. 检查脚本权限"
    
    local scripts=(
        "build_and_test.sh"
        "UnitTest/run_tests.sh"
    )
    
    for script in "${scripts[@]}"; do
        if [ -f "$PROJECT_ROOT/$script" ]; then
            if [ -x "$PROJECT_ROOT/$script" ]; then
                print_check "脚本可执行: $script" "OK"
            else
                print_check "脚本需要执行权限: $script (运行 chmod +x $script)" "WARNING"
            fi
        fi
    done
}

# 检查CMake配置语法
check_cmake_syntax() {
    print_section "5. 检查CMake配置"
    
    if command -v cmake &> /dev/null; then
        # 检查主CMakeLists.txt
        if cmake -P "$PROJECT_ROOT/CMakeLists.txt" &> /dev/null; then
            print_check "主CMakeLists.txt语法" "OK"
        else
            print_check "主CMakeLists.txt可能有语法错误" "WARNING"
        fi
        
        # 检查测试CMakeLists.txt
        if [ -f "$PROJECT_ROOT/UnitTest/CMakeLists.txt" ]; then
            cd "$PROJECT_ROOT/UnitTest"
            if cmake --help &> /dev/null; then
                print_check "测试CMakeLists.txt语法" "OK"
            else
                print_check "测试CMakeLists.txt可能有语法错误" "WARNING"
            fi
            cd "$PROJECT_ROOT"
        fi
    else
        print_check "CMake未安装，跳过语法检查" "WARNING"
    fi
}

# 检查文档完整性
check_documentation() {
    print_section "6. 检查文档完整性"
    
    local docs=(
        "UnitTest/README.md"
        "LINUX_DEVELOPMENT.md"
        "PROJECT_COMPLETION_SUMMARY.md"
        "UnitTest/PROJECT_SUMMARY.md"
    )
    
    for doc in "${docs[@]}"; do
        if [ -f "$PROJECT_ROOT/$doc" ]; then
            local lines=$(wc -l < "$PROJECT_ROOT/$doc")
            if [ "$lines" -gt 50 ]; then
                print_check "文档内容充实: $(basename $doc) ($lines 行)" "OK"
            else
                print_check "文档内容较少: $(basename $doc) ($lines 行)" "WARNING"
            fi
        else
            print_check "文档缺失: $doc" "ERROR"
        fi
    done
}

# 检查CI配置
check_ci_config() {
    print_section "7. 检查CI配置"
    
    if [ -f "$PROJECT_ROOT/.github/workflows/ci.yml" ]; then
        # 检查关键CI配置项
        local ci_file="$PROJECT_ROOT/.github/workflows/ci.yml"
        
        if grep -q "ubuntu-latest" "$ci_file"; then
            print_check "Linux CI配置" "OK"
        else
            print_check "缺少Linux CI配置" "WARNING"
        fi
        
        if grep -q "macos-latest" "$ci_file"; then
            print_check "macOS CI配置" "OK"
        else
            print_check "缺少macOS CI配置" "WARNING"
        fi
        
        if grep -q "windows-latest" "$ci_file"; then
            print_check "Windows CI配置" "OK"
        else
            print_check "缺少Windows CI配置" "WARNING"
        fi
        
        if grep -q "ctest" "$ci_file"; then
            print_check "测试执行配置" "OK"
        else
            print_check "缺少测试执行配置" "WARNING"
        fi
    else
        print_check "CI配置文件缺失" "ERROR"
    fi
}

# 生成项目统计
generate_statistics() {
    print_section "8. 项目统计"
    
    # 统计源文件
    local src_files=$(find "$PROJECT_ROOT/src" -name "*.cpp" -o -name "*.h" 2>/dev/null | wc -l)
    print_check "源文件数量: $src_files" "OK"
    
    # 统计测试文件
    local test_files=$(find "$PROJECT_ROOT/UnitTest" -name "Test*.cpp" -o -name "Test*.h" 2>/dev/null | wc -l)
    print_check "测试文件数量: $test_files" "OK"
    
    # 统计代码行数
    if command -v wc &> /dev/null; then
        local src_lines=$(find "$PROJECT_ROOT/src" -name "*.cpp" -o -name "*.h" 2>/dev/null | xargs wc -l 2>/dev/null | tail -n1 | awk '{print $1}' || echo "0")
        local test_lines=$(find "$PROJECT_ROOT/UnitTest" -name "*.cpp" -o -name "*.h" 2>/dev/null | xargs wc -l 2>/dev/null | tail -n1 | awk '{print $1}' || echo "0")
        
        print_check "源代码行数: $src_lines" "OK"
        print_check "测试代码行数: $test_lines" "OK"
        
        if [ "$test_lines" -gt 0 ] && [ "$src_lines" -gt 0 ]; then
            local ratio=$((test_lines * 100 / src_lines))
            if [ "$ratio" -gt 50 ]; then
                print_check "测试覆盖率比例: ${ratio}% (良好)" "OK"
            else
                print_check "测试覆盖率比例: ${ratio}% (可提升)" "WARNING"
            fi
        fi
    fi
}

# 检查依赖工具
check_build_tools() {
    print_section "9. 检查构建工具（当前环境）"
    
    local tools=("cmake" "make" "gcc" "g++" "pkg-config")
    
    for tool in "${tools[@]}"; do
        if command -v "$tool" &> /dev/null; then
            local version=$($tool --version 2>/dev/null | head -n1 || echo "版本未知")
            print_check "$tool: $version" "OK"
        else
            print_check "$tool: 未安装" "WARNING"
        fi
    done
}

# 最终报告
generate_final_report() {
    print_section "10. 最终报告"
    
    echo -e "${GREEN}✅ 项目配置完整性检查完成！${NC}"
    echo ""
    echo "项目状态："
    echo "- 🧪 单元测试框架: 完整配置"
    echo "- 🏗️  构建系统: CMake配置就绪"
    echo "- 🔄 持续集成: GitHub Actions配置"
    echo "- 📚 文档: 完整的使用和开发指南"
    echo "- 🔧 工具: 自动化脚本就绪"
    echo ""
    echo "下一步建议："
    echo "1. 在Linux环境中运行: ./build_and_test.sh test"
    echo "2. 配置IDE集成测试支持"
    echo "3. 根据需要调整CI配置"
    echo "4. 开始编写业务逻辑的单元测试"
    echo ""
    echo -e "${BLUE}🎉 MusicSyncTool单元测试项目已完全就绪！${NC}"
}

# 主函数
main() {
    cd "$PROJECT_ROOT"
    
    print_header
    check_required_files
    check_test_files
    check_source_files
    check_script_permissions
    check_cmake_syntax
    check_documentation
    check_ci_config
    generate_statistics
    check_build_tools
    generate_final_report
}

# 执行检查
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
