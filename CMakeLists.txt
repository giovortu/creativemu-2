cmake_minimum_required(VERSION 3.21)

if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(_creativemu_vcpkg_candidates)

    if(DEFINED ENV{VCPKG_ROOT} AND NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
        list(APPEND _creativemu_vcpkg_candidates "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    endif()

    if(DEFINED ENV{USERPROFILE} AND NOT "$ENV{USERPROFILE}" STREQUAL "")
        list(APPEND _creativemu_vcpkg_candidates "$ENV{USERPROFILE}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    endif()

    foreach(_creativemu_vcpkg_toolchain IN LISTS _creativemu_vcpkg_candidates)
        if(EXISTS "${_creativemu_vcpkg_toolchain}")
            set(
                CMAKE_TOOLCHAIN_FILE
                "${_creativemu_vcpkg_toolchain}"
                CACHE FILEPATH
                "Path to the vcpkg toolchain file"
            )
            break()
        endif()
    endforeach()

    if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg target triplet")
    endif()
endif()

project(CreatiVemu2 VERSION 0.5.3 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

find_package(SDL2 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

set(CREATIVEMU_SOURCES
    main.cpp
    audio/sn76496.cpp
    cpu/cpu6502.cpp
    font/font.cpp
    keyboard/keyboard.cpp
    mem/cvmemory.cpp
    menu/menu.cpp
    pia/6821pia.cpp
    savestate/savestate.cpp
    video/drv9918.cpp
    video/tms9918.cpp
)

set(CREATIVEMU_RESOURCES)
if(WIN32)
    list(APPEND CREATIVEMU_RESOURCES CreatiVemu_private.rc)
endif()

add_executable(CreatiVemu2 ${CREATIVEMU_SOURCES} ${CREATIVEMU_RESOURCES})

target_include_directories(CreatiVemu2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(CreatiVemu2 PRIVATE SDL_MAIN_HANDLED)

if(WIN32)
    target_compile_definitions(CreatiVemu2 PRIVATE WINDOWS NOMINMAX)
    set_target_properties(
        CreatiVemu2
        PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CreatiVemu2>"
            # Use the GUI subsystem so no console window is shown when the
            # executable is launched. The code keeps a standard int main()
            # (no WinMain), so tell the MSVC linker to still start from the
            # normal CRT/main entry point instead of expecting WinMain.
            WIN32_EXECUTABLE TRUE
    )
    if(MSVC)
        target_link_options(CreatiVemu2 PRIVATE "/ENTRY:mainCRTStartup")
    endif()
endif()

target_link_libraries(CreatiVemu2 PRIVATE SDL2::SDL2 ZLIB::ZLIB)

# Place the built executable directly in the repository's "bin" folder
# (which already holds bios/ and roms/ assets) instead of the default
# build/<Config> layout, on every generator (single- and multi-config).
set(CREATIVEMU_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set_target_properties(
    CreatiVemu2
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CREATIVEMU_RUNTIME_DIR}"
)
foreach(_creativemu_cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${_creativemu_cfg}" _creativemu_cfg_upper)
    set_target_properties(
        CreatiVemu2
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${_creativemu_cfg_upper} "${CREATIVEMU_RUNTIME_DIR}"
    )
endforeach()

if(NOT CREATIVEMU_RUNTIME_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    # Runtime dir differs from the source "bin" folder (e.g. a custom output
    # location): copy the BIOS/ROM assets next to the executable.
    add_custom_command(
        TARGET CreatiVemu2
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CreatiVemu2>/bios"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CreatiVemu2>/roms"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CreatiVemu2>/snapshots"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/bin/bios"
                "$<TARGET_FILE_DIR:CreatiVemu2>/bios"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/bin/roms"
                "$<TARGET_FILE_DIR:CreatiVemu2>/roms"
        COMMENT "Copying BIOS and ROM assets next to the executable"
    )
else()
    # Executable already lands in the source "bin" folder alongside the
    # existing bios/roms assets: just make sure "snapshots" exists.
    add_custom_command(
        TARGET CreatiVemu2
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CREATIVEMU_RUNTIME_DIR}/snapshots"
        COMMENT "Ensuring snapshots folder exists next to the executable"
    )
endif()
