# Unit test CMakeLists.txt for MusicSyncTool
# This file is for Linux and macOS only (matching parent CMakeLists.txt).
# Build with: cmake -DBUILD_TESTS=ON .. && make
# Run with:   ctest
cmake_minimum_required(VERSION 3.16)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_AUTOMOC ON)

    # Qt modules: Widgets (for QApplication in test runner), Test, Sql, Multimedia
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia Test)
    find_package(TagLib REQUIRED)

    # ==================== Main project sources (compiled into test binary) ====================

    # Core layer
    set(MAIN_CORE_SOURCES
        ../src/Core/StringSimilarity.cpp
    )
    set(MAIN_CORE_HEADERS
        ../src/Core/MusicProperties.h
        ../src/Core/SettingEntity.h
        ../src/Core/SingleInstance.h
        ../src/Core/StringSimilarity.h
        ../src/Core/MSTTagUtils.h
    )

    # Data layer
    set(MAIN_DATA_SOURCES
        ../src/Data/QueryItem.cpp
        ../src/Data/LyricIgnoreRule.cpp
    )
    set(MAIN_DATA_HEADERS
        ../src/Data/QueryItem.h
        ../src/Data/LyricIgnoreRule.h
    )

    # Services layer
    set(MAIN_SERVICES_SOURCES
        ../src/Services/Logger.cpp
        ../src/Services/MSTDataSource.cpp
        ../src/Services/MSTFileManager.cpp
        ../src/Services/MSTMediaPlayer.cpp
        ../src/Services/MSTScanController.cpp
        ../src/Services/MSTSettingsManager.cpp
        ../src/Services/MSTTagScanner.cpp
    )
    set(MAIN_SERVICES_HEADERS
        ../src/Services/Logger.h
        ../src/Services/MSTDataSource.h
        ../src/Services/MSTFileManager.h
        ../src/Services/MSTMediaPlayer.h
        ../src/Services/MSTScanController.h
        ../src/Services/MSTSettingsManager.h
        ../src/Services/MSTTagScanner.h
    )

    # All main project sources (excluding UI layer - requires Qt widget instantiation)
    set(MAIN_PROJECT_SOURCES
        ${MAIN_CORE_SOURCES}
        ${MAIN_CORE_HEADERS}
        ${MAIN_DATA_SOURCES}
        ${MAIN_DATA_HEADERS}
        ${MAIN_SERVICES_SOURCES}
        ${MAIN_SERVICES_HEADERS}
    )

    # ==================== Test sources ====================

    # Test headers (listed for AUTOMOC Q_OBJECT detection via main.cpp direct includes)
    set(TEST_HEADERS
        TestMusicProperties.h
        TestStringSimilarity.h
        TestQueryItem.h
        TestLyricIgnoreRule.h
        TestMSTFileManager.h
        TestMSTDataSource.h
        TestMSTSettingsManager.h
        TestBlackBox.h
    )

    # Test implementation files.
    # These are #included by main.cpp (not compiled as separate translation units).
    # They are listed here solely for IDE visibility.
    # AUTOMOC must NOT process them individually (would cause duplicate symbols
    # with main.cpp which includes them directly).
    set(TEST_IMPL_SOURCES
        TestMusicProperties.cpp
        TestStringSimilarity.cpp
        TestQueryItem.cpp
        TestLyricIgnoreRule.cpp
        TestMSTFileManager.cpp
        TestMSTDataSource.cpp
        TestMSTSettingsManager.cpp
        TestBlackBox.cpp
    )

    # ==================== Build test executable ====================

    # main.cpp is the only compiled source. It #includes all Test*.cpp files directly.
    # AUTOMOC processes main.cpp, detects Q_OBJECT in the directly-included test
    # headers, and generates main.moc with all Q_OBJECT implementations.
    # The test impl files are added with SKIP_AUTOMOC to prevent AUTOMOC from
    # processing them (which would cause duplicate symbol errors).
    set_source_files_properties(${TEST_IMPL_SOURCES} PROPERTIES SKIP_AUTOMOC TRUE)

    qt_add_executable(MusicSyncToolTests
        main.cpp
        ${TEST_HEADERS}
        ${TEST_IMPL_SOURCES}
        ${MAIN_PROJECT_SOURCES}
    )

    # Include directories:
    #   - CMAKE_CURRENT_SOURCE_DIR for test headers (TestXxx.h)
    #   - ../src and subdirectories for project headers using short includes
    #   - ../src/UI/MusicSyncTool for MSTTableManager.h (included by MSTScanController.cpp)
    target_include_directories(MusicSyncToolTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Data
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Services
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/UI/MusicSyncTool
    )

    target_link_libraries(MusicSyncToolTests PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Multimedia
        Qt6::Test
        TagLib::tag
    )

    target_compile_options(MusicSyncToolTests PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fPIC>
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    )

    # ==================== CTest integration ====================
    add_test(NAME MusicSyncToolTests COMMAND MusicSyncToolTests)

    # ==================== Optional: Code coverage ====================
    option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
    if(ENABLE_COVERAGE)
        target_compile_options(MusicSyncToolTests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
        target_link_options(MusicSyncToolTests PRIVATE --coverage)
        message(STATUS "Code coverage enabled for unit tests.")
    endif()

    # ==================== Optional: Address sanitizer ====================
    option(ENABLE_SANITIZER "Enable AddressSanitizer" OFF)
    if(ENABLE_SANITIZER)
        target_compile_options(MusicSyncToolTests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(MusicSyncToolTests PRIVATE -fsanitize=address)
        message(STATUS "AddressSanitizer enabled for unit tests.")
    endif()

endif()
