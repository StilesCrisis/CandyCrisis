cmake_minimum_required(VERSION 3.20)
project(CandyCrisis CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Source directory
# ---------------------------------------------------------------------------
set(SRC_DIR    "${CMAKE_SOURCE_DIR}/src")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
set(DEPS_DIR   "${CMAKE_SOURCE_DIR}/deps")

# ---------------------------------------------------------------------------
# Source files
# All .cpp files in the source directory; .mm files are Apple-only and excluded
# by the *.cpp glob pattern.
# ---------------------------------------------------------------------------
file(GLOB SOURCES "${SRC_DIR}/*.cpp")


if(EMSCRIPTEN)

# ---------------------------------------------------------------------------
# Emscripten (WebAssembly) build
# SDL2 and SDL2_image are provided as Emscripten ports.
# Audio is handled entirely by libopenmpt (music) and the Web Audio API (SFX);
# SDL_mixer is not used.
# ASYNCIFY allows the game's blocking while-loops to work in a browser context.
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# libopenmpt — pre-built Emscripten static library included at deps/libopenmpt/.
# Headers are at deps/libopenmpt/include/libopenmpt/.
# License: BSD 3-Clause — see deps/libopenmpt/LICENSE.
#
# To rebuild libopenmpt.a (e.g. for a newer Emscripten version):
#   1. Download the autotools source tarball from lib.openmpt.org/files/libopenmpt/src/
#   2. Extract and in that directory run:
#        source ~/emsdk/emsdk_env.sh
#        emconfigure ./configure --disable-examples --disable-tests \
#            --without-portaudio --without-portaudiocpp --without-sdl2 \
#            --without-pulseaudio --without-mpg123 --without-ogg \
#            --without-vorbis --without-vorbisfile --without-sndfile \
#            --without-flac --without-zlib --disable-dependency-tracking
#        emmake make -j$(sysctl -n hw.logicalcpu)
#   3. Copy .libs/libopenmpt.a to deps/libopenmpt/lib/libopenmpt.a
# ---------------------------------------------------------------------------
set(LIBOPENMPT_LIB     "${DEPS_DIR}/libopenmpt/lib/libopenmpt.a")
set(LIBOPENMPT_INCLUDE "${DEPS_DIR}/libopenmpt/include")

# Use Input_Emscripten.cpp instead of keyselect.cpp, mirroring how the iOS build
# uses Input_iOS.mm in place of keyselect.cpp.
# Input_Emscripten.cpp is already picked up by the glob; just drop keyselect.cpp.
list(REMOVE_ITEM SOURCES "${SRC_DIR}/keyselect.cpp")

# Build ID for cache-busting dynamically-loaded package scripts.
string(TIMESTAMP BUILD_ID "%Y%m%d%H%M%S")

add_executable(CandyCrisis ${SOURCES})
set_target_properties(CandyCrisis PROPERTIES SUFFIX ".html")

# Precompiled header (emcc is Clang-based so PCH works normally)
target_precompile_headers(CandyCrisis PRIVATE "${SRC_DIR}/stdafx.h")

target_include_directories(CandyCrisis PRIVATE
    "${SRC_DIR}"
    "${LIBOPENMPT_INCLUDE}"
)

# -sUSE_SDL=2 / -sUSE_SDL_IMAGE=2 must be on both compile and link lines so
# Emscripten adds the port include paths at compile time and links the port libs.
target_compile_options(CandyCrisis PRIVATE
    -sUSE_SDL=2
    -sUSE_SDL_IMAGE=2
    -Os
    -flto
)

target_compile_definitions(CandyCrisis PRIVATE "BUILD_ID=\"${BUILD_ID}\"")

# Resources are packed into a .data file at link time via --preload-file.
# They must already be present in the build directory before the link step,
# so we copy them in a PRE_LINK command rather than POST_BUILD.
set(WEB_RESOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisisResources")

add_custom_command(TARGET CandyCrisis PRE_LINK
    COMMAND ${CMAKE_COMMAND}
        -D "SRC_DIR=${ASSETS_DIR}"
        -D "DST_DIR=${WEB_RESOURCES_DIR}"
        -P "${CMAKE_SOURCE_DIR}/cmake/CopyCandyCrisisResources.cmake"
    COMMENT "Copying game resources for web preload"
)

target_link_libraries(CandyCrisis PRIVATE "${LIBOPENMPT_LIB}")

target_link_options(CandyCrisis PRIVATE
    -Os
    -flto
    -sUSE_SDL=2
    -sUSE_SDL_IMAGE=2
    "-sSDL2_IMAGE_FORMATS=[\"png\",\"jpg\"]"
    -sASYNCIFY=1
    -sASYNCIFY_STACK_SIZE=65536
    -sALLOW_MEMORY_GROWTH=1
    "-sINITIAL_MEMORY=134217728"
    -sFORCE_FILESYSTEM=1
    # Export the PCM render function called from audio.js ScriptProcessorNode,
    # plus malloc/free used to allocate the render buffers from JS.
    "-sEXPORTED_FUNCTIONS=[\"_main\",\"_CandyCrisisAudio_RenderMusic\",\"_malloc\",\"_free\",\"_Platform_SetTouchTap\",\"_Platform_SetTouchTranslation\"]"
    "-sEXPORTED_RUNTIME_METHODS=[\"HEAPF32\"]"
    "SHELL:--shell-file \"${CMAKE_SOURCE_DIR}/web/shell.html\""
)

# Tell ninja to relink when shell.html changes (it's a linker input but CMake
# doesn't track --shell-file automatically).
set_target_properties(CandyCrisis PROPERTIES
    LINK_DEPENDS "${CMAKE_SOURCE_DIR}/web/shell.html"
)

# Resources are packaged by file_packager.py after the link step.
# Each bundle is a separate ninja step for granular progress reporting.
get_filename_component(EMSCRIPTEN_ROOT "${CMAKE_CXX_COMPILER}" DIRECTORY)
find_program(PYTHON NAMES python3 python REQUIRED)
set(FILE_PACKAGER "${EMSCRIPTEN_ROOT}/tools/file_packager.py")

# Copy web assets alongside the built HTML/JS so the shell can load them.
add_custom_command(TARGET CandyCrisis POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/web/audio.js"
        "${CMAKE_CURRENT_BINARY_DIR}/audio.js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/web/manifest.json"
        "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/web/favicon.png"
        "${CMAKE_CURRENT_BINARY_DIR}/favicon.png"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/web/polka-dot.png"
        "${CMAKE_CURRENT_BINARY_DIR}/polka-dot.png"
    COMMENT "Copying web assets"
)

# ---------------------------------------------------------------------------
# Resource packaging — one ninja step per bundle.
# ---------------------------------------------------------------------------
set(PKG_OUTPUTS)

# Shared bundle (UI, backdrops, music, sounds — everything except PICT_5xxx).
# File list is enumerated at build time by PackageCandyCrisisWeb.cmake.
add_custom_command(
    OUTPUT
        "${CMAKE_CURRENT_BINARY_DIR}/pkg_shared.data"
        "${CMAKE_CURRENT_BINARY_DIR}/pkg_shared.js"
    COMMAND ${CMAKE_COMMAND}
        -D "PYTHON=${PYTHON}"
        -D "FILE_PACKAGER=${FILE_PACKAGER}"
        -D "RESOURCES_DIR=${WEB_RESOURCES_DIR}"
        -D "OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
        -P "${CMAKE_SOURCE_DIR}/cmake/PackageCandyCrisisWeb.cmake"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisis.html"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Packaging shared bundle -> pkg_shared"
    VERBATIM
)
list(APPEND PKG_OUTPUTS
    "${CMAKE_CURRENT_BINARY_DIR}/pkg_shared.data"
    "${CMAKE_CURRENT_BINARY_DIR}/pkg_shared.js"
)

# Per-level character bundles — enumerated from the assets tree at configure time.
file(GLOB CHAR_DIRS_SRC "${ASSETS_DIR}/PICT_5[0-9][0-9][0-9]")
foreach(dir ${CHAR_DIRS_SRC})
    get_filename_component(name "${dir}" NAME)
    set(extra_exclude "")
    # Bundle the level's music file alongside its character assets.
    # For PICT_5001–5011 the music ID equals the character index (1–11),
    # so the file is mod_(index+128). PICT_5000 and PICT_5013 both use
    # music 0 (mod_128), which stays in the shared bundle.
    string(REGEX REPLACE "^PICT_5([0-9]+)$" "\\1" char_idx_raw "${name}")
    math(EXPR char_idx "${char_idx_raw} + 0")  # strip leading zeros
    if(char_idx GREATER_EQUAL 1 AND char_idx LESS_EQUAL 11)
        math(EXPR music_id "${char_idx} + 128")
        set(extra_music "--preload" "CandyCrisisResources/mod_${music_id}@CandyCrisisResources/mod_${music_id}")
    else()
        set(extra_music "")
    endif()
    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/pkg_level_${char_idx}.data"
            "${CMAKE_CURRENT_BINARY_DIR}/pkg_level_${char_idx}.js"
        COMMAND "${PYTHON}" "${FILE_PACKAGER}"
            "${CMAKE_CURRENT_BINARY_DIR}/pkg_level_${char_idx}.data"
            "--js-output=${CMAKE_CURRENT_BINARY_DIR}/pkg_level_${char_idx}.js"
            "--quiet"
            "--preload" "CandyCrisisResources/${name}@CandyCrisisResources/${name}"
            ${extra_exclude}
            ${extra_music}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisis.html"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Packaging level assets: ${name} -> pkg_level_${char_idx}"
        VERBATIM
    )
    list(APPEND PKG_OUTPUTS
        "${CMAKE_CURRENT_BINARY_DIR}/pkg_level_${char_idx}.data"
        "${CMAKE_CURRENT_BINARY_DIR}/pkg_level_${char_idx}.js"
    )
endforeach()

# World-complete bundles — streamed while playing the final level of each world.
# World 1 (levels 1-4) → pkg_world1  (PICT_600-609)
# World 2 (levels 5-8) → pkg_world2  (PICT_610-621)
# World 3 (levels 9-12) → pkg_world3 (PICT_630-649, includes credits + post-credits)
#
# NOTE: PICT_617-621 (World 2 victory layers 6-10) are also included in pkg_world3.
# The World 3 victory screen reuses these layers (see kWorld3Layers in UI_VictoryScreen.cpp)
# but a player can open the page and play World 3 directly without ever loading pkg_world2.
# Duplicating them here ensures they are available. The Emscripten FS silently overwrites
# with identical bytes if pkg_world2 also loaded them.
foreach(w 1 2 3)
    if(w EQUAL 1)
        set(world_preloads
            "--preload" "CandyCrisisResources/PICT_600.jpg@CandyCrisisResources/PICT_600.jpg"
            "--preload" "CandyCrisisResources/PICT_601.png@CandyCrisisResources/PICT_601.png"
            "--preload" "CandyCrisisResources/PICT_602.png@CandyCrisisResources/PICT_602.png"
            "--preload" "CandyCrisisResources/PICT_603.png@CandyCrisisResources/PICT_603.png"
            "--preload" "CandyCrisisResources/PICT_604.png@CandyCrisisResources/PICT_604.png"
            "--preload" "CandyCrisisResources/PICT_605.png@CandyCrisisResources/PICT_605.png"
            "--preload" "CandyCrisisResources/PICT_606.png@CandyCrisisResources/PICT_606.png"
            "--preload" "CandyCrisisResources/PICT_607.png@CandyCrisisResources/PICT_607.png"
            "--preload" "CandyCrisisResources/PICT_608.png@CandyCrisisResources/PICT_608.png"
            "--preload" "CandyCrisisResources/PICT_609.png@CandyCrisisResources/PICT_609.png"
        )
    elseif(w EQUAL 2)
        set(world_preloads
            "--preload" "CandyCrisisResources/PICT_610.jpg@CandyCrisisResources/PICT_610.jpg"
            "--preload" "CandyCrisisResources/PICT_611.png@CandyCrisisResources/PICT_611.png"
            "--preload" "CandyCrisisResources/PICT_612.png@CandyCrisisResources/PICT_612.png"
            "--preload" "CandyCrisisResources/PICT_613.png@CandyCrisisResources/PICT_613.png"
            "--preload" "CandyCrisisResources/PICT_614.png@CandyCrisisResources/PICT_614.png"
            "--preload" "CandyCrisisResources/PICT_615.png@CandyCrisisResources/PICT_615.png"
            "--preload" "CandyCrisisResources/PICT_616.png@CandyCrisisResources/PICT_616.png"
            "--preload" "CandyCrisisResources/PICT_617.png@CandyCrisisResources/PICT_617.png"
            "--preload" "CandyCrisisResources/PICT_618.png@CandyCrisisResources/PICT_618.png"
            "--preload" "CandyCrisisResources/PICT_619.png@CandyCrisisResources/PICT_619.png"
            "--preload" "CandyCrisisResources/PICT_620.png@CandyCrisisResources/PICT_620.png"
            "--preload" "CandyCrisisResources/PICT_621.png@CandyCrisisResources/PICT_621.png"
        )
    else()
        set(world_preloads
            # PICT_617-621: World 2 layers 6-10, shared with World 3 victory screen (see comment above).
            "--preload" "CandyCrisisResources/PICT_617.png@CandyCrisisResources/PICT_617.png"
            "--preload" "CandyCrisisResources/PICT_618.png@CandyCrisisResources/PICT_618.png"
            "--preload" "CandyCrisisResources/PICT_619.png@CandyCrisisResources/PICT_619.png"
            "--preload" "CandyCrisisResources/PICT_620.png@CandyCrisisResources/PICT_620.png"
            "--preload" "CandyCrisisResources/PICT_621.png@CandyCrisisResources/PICT_621.png"
            "--preload" "CandyCrisisResources/PICT_630.jpg@CandyCrisisResources/PICT_630.jpg"
            "--preload" "CandyCrisisResources/PICT_631.png@CandyCrisisResources/PICT_631.png"
            "--preload" "CandyCrisisResources/PICT_632.png@CandyCrisisResources/PICT_632.png"
            "--preload" "CandyCrisisResources/PICT_633.png@CandyCrisisResources/PICT_633.png"
            "--preload" "CandyCrisisResources/PICT_634.png@CandyCrisisResources/PICT_634.png"
            "--preload" "CandyCrisisResources/PICT_635.png@CandyCrisisResources/PICT_635.png"
            "--preload" "CandyCrisisResources/PICT_636.png@CandyCrisisResources/PICT_636.png"
            "--preload" "CandyCrisisResources/PICT_637.png@CandyCrisisResources/PICT_637.png"
            "--preload" "CandyCrisisResources/PICT_638.png@CandyCrisisResources/PICT_638.png"
            "--preload" "CandyCrisisResources/PICT_639.png@CandyCrisisResources/PICT_639.png"
            "--preload" "CandyCrisisResources/PICT_640.png@CandyCrisisResources/PICT_640.png"
            "--preload" "CandyCrisisResources/PICT_641.png@CandyCrisisResources/PICT_641.png"
            "--preload" "CandyCrisisResources/PICT_642.png@CandyCrisisResources/PICT_642.png"
            "--preload" "CandyCrisisResources/PICT_643.png@CandyCrisisResources/PICT_643.png"
            "--preload" "CandyCrisisResources/PICT_644.png@CandyCrisisResources/PICT_644.png"
            "--preload" "CandyCrisisResources/PICT_645.png@CandyCrisisResources/PICT_645.png"
            "--preload" "CandyCrisisResources/PICT_646.png@CandyCrisisResources/PICT_646.png"
            "--preload" "CandyCrisisResources/PICT_647.png@CandyCrisisResources/PICT_647.png"
            "--preload" "CandyCrisisResources/PICT_648.png@CandyCrisisResources/PICT_648.png"
            "--preload" "CandyCrisisResources/PICT_649.png@CandyCrisisResources/PICT_649.png"
        )
    endif()
    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/pkg_world${w}.data"
            "${CMAKE_CURRENT_BINARY_DIR}/pkg_world${w}.js"
        COMMAND "${PYTHON}" "${FILE_PACKAGER}"
            "${CMAKE_CURRENT_BINARY_DIR}/pkg_world${w}.data"
            "--js-output=${CMAKE_CURRENT_BINARY_DIR}/pkg_world${w}.js"
            "--quiet"
            ${world_preloads}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisis.html"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Packaging world ${w} complete assets -> pkg_world${w}"
        VERBATIM
    )
    list(APPEND PKG_OUTPUTS
        "${CMAKE_CURRENT_BINARY_DIR}/pkg_world${w}.data"
        "${CMAKE_CURRENT_BINARY_DIR}/pkg_world${w}.js"
    )
endforeach()

add_custom_target(WebPackages ALL DEPENDS ${PKG_OUTPUTS})
add_dependencies(WebPackages CandyCrisis)

# Collect all deployable files into a clean dist/ folder.
set(DIST_DIR "${CMAKE_CURRENT_BINARY_DIR}/dist" CACHE PATH "Deployable output directory")
add_custom_target(WebDist ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisis.html"
        "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisis.js"
        "${CMAKE_CURRENT_BINARY_DIR}/CandyCrisis.wasm"
        "${CMAKE_CURRENT_BINARY_DIR}/audio.js"
        "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
        "${CMAKE_CURRENT_BINARY_DIR}/favicon.png"
        "${CMAKE_CURRENT_BINARY_DIR}/polka-dot.png"
        ${PKG_OUTPUTS}
        "${DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/web/index.html"
        "${DIST_DIR}/index.html"
    DEPENDS ${PKG_OUTPUTS}
    COMMENT "Collecting deployable files into dist/"
    VERBATIM
)
add_dependencies(WebDist WebPackages CandyCrisis)

elseif(APPLE)

# ---------------------------------------------------------------------------
# macOS (CMake / SDL2) build
#
# Dependencies:
#   brew install sdl2 sdl2_image
#
#   FMOD Core API for macOS — download from fmod.com and place at:
#     deps/FMOD/api/core/inc/     (fmod.hpp, fmod_errors.h, etc.)
#     deps/FMOD/api/core/lib/     (libfmod.dylib)
# ---------------------------------------------------------------------------

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

set(FMOD_INC "${DEPS_DIR}/FMOD/api/core/inc")
set(FMOD_LIB "${DEPS_DIR}/FMOD/api/core/lib/libfmod.dylib")

# Use SDL keyboard input; drop Emscripten touch input.
list(REMOVE_ITEM SOURCES "${SRC_DIR}/Input_Emscripten.cpp")

add_executable(CandyCrisis ${SOURCES})

target_precompile_headers(CandyCrisis PRIVATE "${SRC_DIR}/stdafx.h")

target_include_directories(CandyCrisis PRIVATE
    "${SRC_DIR}"
    "${FMOD_INC}"
)

target_link_libraries(CandyCrisis PRIVATE
    SDL2::SDL2
    SDL2_image::SDL2_image
    "${FMOD_LIB}"
    "-framework Foundation"
    "-framework AppKit"
)

# Copy libfmod.dylib next to the executable after each build.
add_custom_command(TARGET CandyCrisis POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FMOD_LIB}"
        $<TARGET_FILE_DIR:CandyCrisis>
    COMMENT "Copying libfmod.dylib to output directory"
)

# Copy game resources next to the executable after each build.
add_custom_command(TARGET CandyCrisis POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D "SRC_DIR=${ASSETS_DIR}"
        -D "DST_DIR=$<TARGET_FILE_DIR:CandyCrisis>/CandyCrisisResources"
        -P "${CMAKE_SOURCE_DIR}/cmake/CopyCandyCrisisResources.cmake"
    COMMENT "Copying game resources to output directory"
)

else()

# ---------------------------------------------------------------------------
# Windows (MSVC) build
# Dependency paths: place downloaded SDKs under deps/ at the project root:
#
#   deps/
#     SDL2/                 ← contents of SDL2-devel-X.Y.Z-VC.zip
#       include/
#       lib/x64/
#     SDL2_image/           ← contents of SDL2_image-devel-X.Y.Z-VC.zip
#       include/
#       lib/x64/
#     FMOD/                 ← installed FMOD Core API for Windows
#       api/
#         core/
#           inc/
#           lib/
#             x64/
# ---------------------------------------------------------------------------

set(SDL2_INC        "${DEPS_DIR}/SDL2/include")
set(SDL2_LIB_DIR    "${DEPS_DIR}/SDL2/lib/x64")

set(SDL2_IMAGE_INC      "${DEPS_DIR}/SDL2_image/include")
set(SDL2_IMAGE_LIB_DIR  "${DEPS_DIR}/SDL2_image/lib/x64")

set(FMOD_INC        "${DEPS_DIR}/FMOD/api/core/inc")
set(FMOD_LIB_DIR    "${DEPS_DIR}/FMOD/api/core/lib/x64")

add_executable(CandyCrisis WIN32 ${SOURCES})

# Precompiled header — stdafx.h is included by every .cpp via /FI (forced include).
# This compiles windows.h, SDL.h, etc. once instead of per-TU.
target_precompile_headers(CandyCrisis PRIVATE "${SRC_DIR}/stdafx.h")

target_include_directories(CandyCrisis PRIVATE
    "${SRC_DIR}"
    "${SDL2_INC}"
    "${SDL2_IMAGE_INC}"
    "${FMOD_INC}"
)

target_compile_definitions(CandyCrisis PRIVATE
    _WIN32
    WIN32
    _WINDOWS
)

target_link_libraries(CandyCrisis PRIVATE
    "${SDL2_LIB_DIR}/SDL2main.lib"
    "${SDL2_LIB_DIR}/SDL2.lib"
    "${SDL2_IMAGE_LIB_DIR}/SDL2_image.lib"
    "${FMOD_LIB_DIR}/fmod_vc.lib"
    winmm
    advapi32    # Registry access used by prefs.cpp
    shell32
)

# Copy DLLs next to the executable after each build
add_custom_command(TARGET CandyCrisis POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIB_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:CandyCrisis>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_IMAGE_LIB_DIR}/SDL2_image.dll"
        $<TARGET_FILE_DIR:CandyCrisis>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FMOD_LIB_DIR}/fmod.dll"
        $<TARGET_FILE_DIR:CandyCrisis>
    COMMENT "Copying runtime DLLs to output directory"
)

# Copy game resources next to the executable after each build.
add_custom_command(TARGET CandyCrisis POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D "SRC_DIR=${ASSETS_DIR}"
        -D "DST_DIR=$<TARGET_FILE_DIR:CandyCrisis>/CandyCrisisResources"
        -P "${CMAKE_SOURCE_DIR}/cmake/CopyCandyCrisisResources.cmake"
    COMMENT "Copying game resources to output directory"
)

endif()
