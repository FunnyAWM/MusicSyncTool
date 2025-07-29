# MusicSyncTool Unit Tests CMakeLists.txt
# Based on main project configuration for consistency

cmake_minimum_required(VERSION 3.16)
project(MusicSyncToolTests VERSION 2.2.3 LANGUAGES CXX)

# Check if this is being run on supported systems
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Supported system detected for unit tests. Applying settings.")
    
    # Set Qt automation (same as main project)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    # Set C++ standard (same as main project)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Enable testing
    enable_testing()
    
    # Qt installation path - try to detect or use environment variable
    # This should match the main project configuration
    if(DEFINED ENV{Qt6_DIR})
        set(CMAKE_PREFIX_PATH $ENV{Qt6_DIR})
        message(STATUS "Using Qt6_DIR from environment: $ENV{Qt6_DIR}")
    elseif(EXISTS "/home/funnyawm/Qt/6.8.3/gcc_64")
        set(CMAKE_PREFIX_PATH "/home/funnyawm/Qt/6.8.3/gcc_64")
        message(STATUS "Using default Qt path: /home/funnyawm/Qt/6.8.3/gcc_64")
    else()
        message(STATUS "Using system Qt installation")
    endif()

    # Find packages (same pattern as main project)
    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia Test)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Multimedia Test)
    find_package(ZLIB)
    find_package(TagLib REQUIRED)

    # Source files from main project (relative to parent directory)
    set(MAIN_PROJECT_SOURCES
        ../src/Logger.cpp
        ../src/LyricIgnoreRule.cpp
        ../src/MSTDataSource.cpp
        ../src/MSTFileManager.cpp
        ../src/QueryItem.cpp
        ../src/MSTMediaPlayer.cpp
    )

    # Test source files
    set(TEST_SOURCES
        TestMSTFileManager.cpp
        TestMSTDataSource.cpp
        TestQueryItem.cpp
        TestLyricIgnoreRule.cpp
        TestLogger.cpp
        main.cpp
    )

    # Create test executable (following main project pattern)
    if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(MusicSyncToolTests
            MANUAL_FINALIZATION
            ${TEST_SOURCES}
            ${MAIN_PROJECT_SOURCES}
        )
    else()
        add_executable(MusicSyncToolTests
            ${TEST_SOURCES}
            ${MAIN_PROJECT_SOURCES}
        )
    endif()

    # Link libraries (same pattern as main project)
    target_link_libraries(MusicSyncToolTests PRIVATE 
        Qt6::Core 
        Qt6::Widgets 
        Qt6::Sql 
        Qt6::Multimedia 
        Qt6::Test
    )
    target_link_libraries(MusicSyncToolTests PRIVATE TagLib::tag)

    # Set target properties (similar to main project)
    set_target_properties(MusicSyncToolTests PROPERTIES
        MACOSX_BUNDLE FALSE
        WIN32_EXECUTABLE FALSE
    )

    # Include directories (same structure as main project)
    target_include_directories(MusicSyncToolTests PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/AboutPage
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/AddRuleWidget
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/LoadingPage
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/MusicSyncTool
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/OperationResult
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/Settings
        ${CMAKE_CURRENT_SOURCE_DIR}/../src/Widgets/ShowDupe
    )

    # Compile options (same as main project)
    target_compile_options(MusicSyncToolTests PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fPIC>
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    )

    # Add definitions for proper compilation
    target_compile_definitions(MusicSyncToolTests PRIVATE
        QT_NO_KEYWORDS
        QT_TESTCASE_BUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
    )

    # Qt finalization (if Qt6)
    if (QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(MusicSyncToolTests)
    endif()

    # Add tests to CTest
    add_test(NAME UnitTests COMMAND MusicSyncToolTests)
    add_test(NAME UnitTests_Verbose COMMAND MusicSyncToolTests -v2)

    # Set test properties
    set_tests_properties(UnitTests UnitTests_Verbose PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Optional: Add coverage support
    option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
    if(ENABLE_COVERAGE)
        target_compile_options(MusicSyncToolTests PRIVATE
            --coverage
            -fprofile-arcs
            -ftest-coverage
        )
        target_link_options(MusicSyncToolTests PRIVATE --coverage)
        message(STATUS "Coverage reporting enabled for tests")
    endif()

    # Optional: Add sanitizer support
    option(ENABLE_SANITIZER "Enable address sanitizer" OFF)
    if(ENABLE_SANITIZER)
        target_compile_options(MusicSyncToolTests PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
        )
        target_link_options(MusicSyncToolTests PRIVATE -fsanitize=address)
        message(STATUS "Address sanitizer enabled for tests")
    endif()

    message(STATUS "Unit tests configured successfully")
    message(STATUS "Run 'make test' or 'ctest' to execute tests")

else()
    message(SEND_ERROR "Unit tests are for Linux and MacOS only. For Windows please use Visual Studio.")
    message(SEND_ERROR "Or you can modify this file if you know what you are doing.")
endif()
