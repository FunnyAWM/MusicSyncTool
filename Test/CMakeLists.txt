cmake_minimum_required(VERSION 3.16)

# 以 Test/ 为顶层构建（CI 用法）时补齐 project() 与 CTest；作为 add_subdirectory 子目录时沿用根工程
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(MusicSyncToolTests LANGUAGES CXX)
    enable_testing()
endif()

# TagLib 在部分发行版只装 pkg-config 文件，复用仓库根目录的查找模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_AUTOMOC ON)

    # Concurrent 显式声明：测试目标将来纳入 QtConcurrent 源码时不会再报 QtConcurrentRun not found
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia Concurrent Test)
    find_package(TagLib REQUIRED)

    # Main project sources (compiled into test binary)
    # 不含 MSTScanController：它在 GUI 线程调用 MSTTableManager，测试目标不链接 UI 层，
    # 与 Test/MusicSyncToolTests.vcxproj 的源文件清单保持一致
    set(MAIN_SOURCES
        ../src/Core/StringSimilarity.cpp
        ../src/Data/QueryItem.cpp
        ../src/Data/LyricIgnoreRule.cpp
        ../src/Services/Logger.cpp
        ../src/Services/MSTDataSource.cpp
        ../src/Services/MSTFileManager.cpp
        ../src/Services/MSTMediaPlayer.cpp
        ../src/Services/MSTSettingsManager.cpp
    )

    set(MAIN_HEADERS
        ../src/Core/MusicProperties.h
        ../src/Core/SettingEntity.h
        ../src/Core/SingleInstance.h
        ../src/Core/StringSimilarity.h
        ../src/Core/MSTTagUtils.h
        ../src/Data/QueryItem.h
        ../src/Data/LyricIgnoreRule.h
        ../src/Services/Logger.h
        ../src/Services/MSTDataSource.h
        ../src/Services/MSTFileManager.h
        ../src/Services/MSTMediaPlayer.h
        ../src/Services/MSTSettingsManager.h
    )

    # Test headers
    set(TEST_HEADERS
        TestMusicProperties.h
        TestStringSimilarity.h
        TestQueryItem.h
        TestLyricIgnoreRule.h
        TestMSTFileManager.h
        TestMSTDataSource.h
        TestMSTSettingsManager.h
        TestIntegration.h
        TestBlackBox.h
    )

    # Test implementations (included by main.cpp, not compiled separately)
    set(TEST_IMPL
        TestMusicProperties.cpp
        TestStringSimilarity.cpp
        TestQueryItem.cpp
        TestLyricIgnoreRule.cpp
        TestMSTFileManager.cpp
        TestMSTDataSource.cpp
        TestMSTSettingsManager.cpp
        TestIntegration.cpp
        TestBlackBox.cpp
    )

    set_source_files_properties(${TEST_IMPL} PROPERTIES SKIP_AUTOMOC TRUE)

    qt_add_executable(MusicSyncToolTests
        main.cpp
        ${TEST_HEADERS}
        ${TEST_IMPL}
        ${MAIN_SOURCES}
        ${MAIN_HEADERS}
    )

    target_include_directories(MusicSyncToolTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Data
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Services
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/UI/MusicSyncTool
    )

    target_link_libraries(MusicSyncToolTests PRIVATE
        Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Multimedia Qt6::Concurrent Qt6::Test
        TagLib::tag
    )

    target_compile_options(MusicSyncToolTests PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fPIC>
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    )

    add_test(NAME MusicSyncToolTests COMMAND MusicSyncToolTests)

    option(ENABLE_COVERAGE "Enable code coverage" OFF)
    if(ENABLE_COVERAGE)
        target_compile_options(MusicSyncToolTests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(MusicSyncToolTests PRIVATE --coverage)
    endif()

    option(ENABLE_SANITIZER "Enable AddressSanitizer" OFF)
    if(ENABLE_SANITIZER)
        target_compile_options(MusicSyncToolTests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(MusicSyncToolTests PRIVATE -fsanitize=address)
    endif()
endif()