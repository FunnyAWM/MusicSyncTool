cmake_minimum_required(VERSION 3.16)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia Test)
    find_package(TagLib REQUIRED)

    # Main project sources (compiled into test binary)
    set(MAIN_SOURCES
        ../src/Core/StringSimilarity.cpp
        ../src/Data/QueryItem.cpp
        ../src/Data/LyricIgnoreRule.cpp
        ../src/Services/Logger.cpp
        ../src/Services/MSTDataSource.cpp
        ../src/Services/MSTFileManager.cpp
        ../src/Services/MSTMediaPlayer.cpp
        ../src/Services/MSTScanController.cpp
        ../src/Services/MSTSettingsManager.cpp
        ../src/Services/MSTTagScanner.cpp
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
        ../src/Services/MSTScanController.h
        ../src/Services/MSTSettingsManager.h
        ../src/Services/MSTTagScanner.h
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
        Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Multimedia Qt6::Test
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