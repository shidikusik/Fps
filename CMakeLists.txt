cmake_minimum_required(VERSION 3.16)
project(bloodrush CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(BLOODRUSH_SOURCES
    src/main.cpp
    src/player.cpp
    src/arena.cpp
    src/enemies.cpp
    src/weapons.cpp
    src/particles.cpp
    src/style_meter.cpp
    src/sounds.cpp
    src/cutscene.cpp
    src/touch.cpp
)

# --- raylib: system package when available, otherwise fetch and build ---
if(ANDROID)
    set(raylib_FOUND FALSE)
    set(PLATFORM "Android" CACHE STRING "" FORCE) # raylib platform switch
elseif(CMAKE_CROSSCOMPILING)
    set(raylib_FOUND FALSE) # never use the host's raylib for a foreign target
else()
    find_package(raylib 5.0 QUIET)
endif()

if(NOT raylib_FOUND)
    message(STATUS "System raylib not found — fetching raylib 5.5")
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    if(NOT ANDROID)
        # Build GLFW with native Wayland when the dev headers are available,
        # alongside X11 — GLFW 3.4 picks the backend at runtime. Without the
        # headers we stay X11-only, which still runs on Wayland via XWayland.
        if(UNIX AND NOT APPLE)
            set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                pkg_check_modules(WAYLAND_DEPS QUIET wayland-client wayland-cursor wayland-egl wayland-protocols xkbcommon)
                if(WAYLAND_DEPS_FOUND)
                    message(STATUS "Wayland headers found — enabling native Wayland backend")
                    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
                endif()
            endif()
        endif()
    endif()

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

if(ANDROID)
    # NativeActivity loads libbloodrush.so; raylib provides the glue.
    add_library(bloodrush SHARED ${BLOODRUSH_SOURCES})
    target_link_options(bloodrush PRIVATE "-Wl,-u,ANativeActivity_onCreate")
    target_link_libraries(bloodrush PRIVATE raylib log android EGL GLESv2 OpenSLES m)
else()
    add_executable(bloodrush ${BLOODRUSH_SOURCES})
    target_link_libraries(bloodrush PRIVATE raylib)

    if(UNIX AND NOT APPLE)
        target_link_libraries(bloodrush PRIVATE m)
    endif()

    if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # MinGW: standalone exe, no runtime DLLs, no console window
        target_link_options(bloodrush PRIVATE -static -static-libgcc -static-libstdc++)
        set_target_properties(bloodrush PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()

    # Desktop integration (used by the Flatpak; harmless for local builds)
    include(GNUInstallDirs)
    install(TARGETS bloodrush RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES data/io.github.shidikusik.Bloodrush.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES data/io.github.shidikusik.Bloodrush.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
    install(FILES data/io.github.shidikusik.Bloodrush.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()
