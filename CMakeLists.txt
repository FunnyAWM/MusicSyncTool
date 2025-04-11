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
    set(CMAKE_PREFIX_PATH "/home/funnyawm/Qt/6.8.2/gcc_64")

    find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Sql Multimedia Concurrent)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Multimedia Concurrent)
    find_package(ZLIB)
    find_package(TagLib REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
    set(PROJECT_SOURCES
            src/main.cpp
            src/AboutPage.cpp
            src/LoadingPage.cpp
            src/MusicSyncTool.cpp
            src/ShowDupe.cpp
            src/Settings.cpp
            src/OperationResult.cpp
            src/AddRuleWidget.cpp
            src/LyricIgnoreRule.cpp
            src/Settings.h
            src/AboutPage.h
            src/LoadingPage.h
            src/MusicSyncTool.h
            src/ShowDupe.h
            src/OperationResult.h
            src/MusicProperties.h
            src/SettingEntity.h
            src/AddRuleWidget.h
            src/LyricIgnoreRule.h
            src/AboutPage.ui
            src/LoadingPage.ui
            src/MusicSyncTool.ui
            src/Settings.ui
            src/ShowDupe.ui
            src/OperationResult.ui
            src/AddRuleWidget.ui
            resources/MusicSyncTool.qrc
            resources/MusicSyncTool.rc
            src/SingleInstance.h
            src/MSTDataSource.cpp
            src/MSTDataSource.h
            src/MSTMediaPlayer.cpp
            src/MSTMediaPlayer.h
            src/QueryItem.cpp
            src/QueryItem.h
            src/Logger.cpp
            src/Logger.h
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
else ()
    message(SEND_ERROR "This file is for Linux and MacOS only. For Windows please use Visual Studio.")
    message(SEND_ERROR "Or you can modify this file if you know what you are doing.")
endif ()
