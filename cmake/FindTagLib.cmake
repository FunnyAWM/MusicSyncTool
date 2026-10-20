#[=======================================================================[.rst:
FindTagLib
----------

查找 TagLib，并统一提供 ``TagLib::tag`` 目标。

搜索顺序：

1. TagLib 官方 / vcpkg 提供的 config 包（``TagLibConfig.cmake``、``taglib-config.cmake``）；
2. pkg-config（Debian/Ubuntu 的 ``libtag1-dev`` 只提供 ``taglib.pc``，没有 CMake config，
   这是 CI 上 ``find_package(TagLib)`` 失败的原因）；
3. 手动 ``find_path`` + ``find_library`` 兜底。

结果变量：``TagLib_FOUND``、``TagLib_VERSION``。
提供目标：``TagLib::tag``。
#]=======================================================================]

# 先找官方 config 包：vcpkg 与 Homebrew 都装了它，此时直接复用它定义的 target
if(NOT TagLib_FOUND)
    find_package(TagLib CONFIG QUIET)
endif()

# 没有 config 包时用 pkg-config（Ubuntu 的 libtag1-dev 只有 .pc 文件）
if(NOT TagLib_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(TAGLIB QUIET IMPORTED_TARGET taglib)
    endif()
    if(TAGLIB_FOUND AND NOT TARGET TagLib::tag)
        add_library(TagLib::tag INTERFACE IMPORTED)
        set_target_properties(TagLib::tag PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${TAGLIB_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES PkgConfig::TAGLIB
        )
        set(TagLib_VERSION "${TAGLIB_VERSION}")
        set(TagLib_FOUND TRUE)
    endif()
endif()

# 兜底：手动查找头文件与库
if(NOT TagLib_FOUND)
    find_path(TagLib_INCLUDE_DIR
        NAMES taglib/tag.h
        PATH_SUFFIXES include
    )
    find_library(TagLib_LIBRARY
        NAMES tag taglib
    )
    if(TagLib_INCLUDE_DIR AND TagLib_LIBRARY)
        add_library(TagLib::tag UNKNOWN IMPORTED)
        set_target_properties(TagLib::tag PROPERTIES
            IMPORTED_LOCATION "${TagLib_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${TagLib_INCLUDE_DIR}"
        )
        set(TagLib_FOUND TRUE)
    endif()
endif()

include(FindPackageHandleStandardArgs)
# 只要 TagLib::tag 存在就算找到，具体路径交给 target 承载
if(TARGET TagLib::tag)
    set(TagLib_FOUND TRUE)
endif()
find_package_handle_standard_args(TagLib
    REQUIRED_VARS TagLib_FOUND
    VERSION_VAR TagLib_VERSION
    REASON_FAILURE_MESSAGE "install libtag1-dev (ships only taglib.pc) or provide TagLibConfig.cmake via vcpkg/Homebrew"
)

mark_as_advanced(TagLib_INCLUDE_DIR TagLib_LIBRARY)
