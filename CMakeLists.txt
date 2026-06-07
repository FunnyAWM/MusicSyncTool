#This CMakeLists.txt is for Linux and Mac OS only
cmake_minimum_required(VERSION 3.16)
project(MusicSyncTool VERSION 2.2.3 LANGUAGES CXX)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Supported system detected. Applying settings.")
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    #[[ Replace this with your own Qt installation path
        If you are using distros with Qt6 library packages available(like openSUSE),
        Just install the package and ignore this set statement]]
    set(CMAKE_PREFIX_PATH "/home/funnyawm/Qt/6.8.3/gcc_64")

    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Sql Multimedia Concurrent)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Multimedia Concurrent)
    find_package(ZLIB)
    find_package(TagLib REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS LinguistTools)

    # ==================== 项目源文件 ====================
    # Core层 - 核心基础类型与工具
    set(CORE_SOURCES
            src/Core/StringSimilarity.cpp
    )
    set(CORE_HEADERS
            src/Core/MusicProperties.h
            src/Core/SettingEntity.h
            src/Core/SingleInstance.h
            src/Core/StringSimilarity.h
            src/Core/MSTTagUtils.h
    )

    # Data层 - 数据模型
    set(DATA_SOURCES
            src/Data/QueryItem.cpp
            src/Data/LyricIgnoreRule.cpp
    )
    set(DATA_HEADERS
            src/Data/QueryItem.h
            src/Data/LyricIgnoreRule.h
    )

    # Services层 - 业务逻辑
    set(SERVICES_SOURCES
            src/Services/Logger.cpp
            src/Services/MSTDataSource.cpp
            src/Services/MSTFileManager.cpp
            src/Services/MSTMediaPlayer.cpp
            src/Services/MSTScanController.cpp
            src/Services/MSTSettingsManager.cpp
            src/Services/MSTTagScanner.cpp
    )
    set(SERVICES_HEADERS
            src/Services/Logger.h
            src/Services/MSTDataSource.h
            src/Services/MSTFileManager.h
            src/Services/MSTMediaPlayer.h
            src/Services/MSTScanController.h
            src/Services/MSTSettingsManager.h
            src/Services/MSTTagScanner.h
    )

    # UI层 - 用户界面组件
    set(UI_SOURCES
            src/UI/AboutPage/AboutPage.cpp
            src/UI/AddRuleWidget/AddRuleWidget.cpp
            src/UI/LoadingPage/LoadingPage.cpp
            src/UI/MusicSyncTool/MSTErrorReporter.cpp
            src/UI/MusicSyncTool/MSTMainWindow.cpp
            src/UI/MusicSyncTool/MSTMediaController.cpp
            src/UI/MusicSyncTool/MSTTableManager.cpp
            src/UI/OperationResult/OperationResult.cpp
            src/UI/Settings/Settings.cpp
            src/UI/ShowDupe/ShowDupe.cpp
    )
    set(UI_HEADERS
            src/UI/AboutPage/AboutPage.h
            src/UI/AddRuleWidget/AddRuleWidget.h
            src/UI/LoadingPage/LoadingPage.h
            src/UI/MusicSyncTool/MSTErrorReporter.h
            src/UI/MusicSyncTool/MSTMainWindow.h
            src/UI/MusicSyncTool/MSTMediaController.h
            src/UI/MusicSyncTool/MSTTableManager.h
            src/UI/OperationResult/OperationResult.h
            src/UI/Settings/Settings.h
            src/UI/ShowDupe/ShowDupe.h
    )
    set(UI_FORMS
            src/UI/AboutPage/AboutPage.ui
            src/UI/AddRuleWidget/AddRuleWidget.ui
            src/UI/LoadingPage/LoadingPage.ui
            src/UI/MusicSyncTool/MSTMainWindow.ui
            src/UI/OperationResult/OperationResult.ui
            src/UI/Settings/Settings.ui
            src/UI/ShowDupe/ShowDupe.ui
    )

    # 入口文件
    set(ENTRY_SOURCES
            src/main.cpp
    )

    # 合并所有源文件
    set(PROJECT_SOURCES
            ${CORE_SOURCES}
            ${CORE_HEADERS}
            ${DATA_SOURCES}
            ${DATA_HEADERS}
            ${SERVICES_SOURCES}
            ${SERVICES_HEADERS}
            ${UI_SOURCES}
            ${UI_HEADERS}
            ${UI_FORMS}
            ${ENTRY_SOURCES}
    )

    if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(MusicSyncTool
                MANUAL_FINALIZATION
                ${PROJECT_SOURCES}
        )

        qt_add_translations(MusicSyncTool TS_FILES translations/Translation_en_US.ts translations/Translation_Wenyan.ts translations/Translation_zh_Hans.ts)
        #[[     Define target properties for Android with Qt 6 as:
                set_property(TARGET MusicSyncTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
                             ${CMAKE_CURRENT_SOURCE_DIR}/android)
                For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation]]
    else ()
        if (ANDROID)
            add_library(MusicSyncTool SHARED
                    ${PROJECT_SOURCES}
            )
            #[[         Define properties for Android with Qt 5 after find_package() calls as:
                        set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")]]
        else ()
            add_executable(MusicSyncTool
                    ${PROJECT_SOURCES}
            )
        endif ()
    endif ()

    target_link_libraries(MusicSyncTool PRIVATE Qt6::Widgets Qt6::Sql Qt6::Multimedia Qt6::Concurrent)
    target_link_libraries(MusicSyncTool PRIVATE TagLib::tag)

    #[[  Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
         If you are developing for iOS or macOS you should consider setting an
         explicit, fixed bundle identifier manually though.]]
    if (${QT_VERSION} VERSION_LESS 6.1.0)
        set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MusicSyncTool)
    endif ()
    set_target_properties(MusicSyncTool PROPERTIES
            ${BUNDLE_ID_OPTION}
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
    )

    include(GNUInstallDirs)
    install(TARGETS MusicSyncTool
            BUNDLE DESTINATION .
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    if (QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(MusicSyncTool)
    endif ()
    add_custom_command(TARGET MusicSyncTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:MusicSyncTool>/resources"
            COMMENT "Copying file post-build"
    )
    add_custom_command(TARGET MusicSyncTool POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/translations"
            "$<TARGET_FILE_DIR:MusicSyncTool>/translations"
            COMMENT "Copying translation files"
    )
    
    # 添加头文件包含路径（按模块层级组织）
    target_include_directories(MusicSyncTool PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Data
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Services
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/AboutPage
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/AddRuleWidget
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/LoadingPage
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/MusicSyncTool
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/OperationResult
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Settings
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UI/ShowDupe
    )
    
    # 添加编译选项以确保兼容性
    target_compile_options(MusicSyncTool PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fPIC>
        $<$<COMPILE_LANGUAGE:CXX>:-Wall>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    )
    
    # 可选：添加单元测试支持
    option(BUILD_TESTS "Build unit tests" OFF)
    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(UnitTest)
        message(STATUS "Unit tests enabled. Use 'make test' or 'ctest' to run tests.")
    endif()
    
else ()
    message(SEND_ERROR "This file is for Linux and MacOS only. For Windows please use Visual Studio.")
    message(SEND_ERROR "Or you can modify this file if you know what you are doing.")
endif ()
