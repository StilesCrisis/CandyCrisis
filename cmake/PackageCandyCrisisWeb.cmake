# PackageCandyCrisisWeb.cmake
#
# Packages the shared bundle (UI, backdrops, music, sounds — everything in
# CandyCrisisResources except PICT_5xxx character directories).
# Per-level character bundles are packaged inline in CMakeLists.txt so that
# each appears as a separate ninja build step.
#
# Arguments (passed via -D from CMakeLists.txt):
#   PYTHON         - Python executable path
#   FILE_PACKAGER  - path to Emscripten's file_packager.py
#   RESOURCES_DIR  - absolute path to CandyCrisisResources in the build tree
#   OUTPUT_DIR     - directory where .data/.js files are written

cmake_minimum_required(VERSION 3.20)

# Partition resources: PICT_5xxx dirs vs. everything else.
file(GLOB ALL_ITEMS "${RESOURCES_DIR}/*")

set(CHAR_DIRS)
set(OTHER_ITEMS)

foreach(item ${ALL_ITEMS})
    get_filename_component(name "${item}" NAME)
    if(name MATCHES "^PICT_5[0-9][0-9][0-9]$")
        list(APPEND CHAR_DIRS "${item}")
    elseif(name MATCHES "^mod_1(29|3[0-9])$")
        # mod_129–mod_139 (music 1–11) are bundled with their per-level character packages
    elseif(name MATCHES "^PICT_6[0-4][0-9]\.(png|jpg)$")
        # PICT_600-649 (world-complete screens) are bundled into pkg_world1/2/3
    else()
        list(APPEND OTHER_ITEMS "${item}")
    endif()
endforeach()

# Use relative paths for --preload src@dst so that paths containing '@'
# in the directory name don't confuse file_packager.py, which splits on
# the first '@' to find the src/dst boundary.
get_filename_component(RESOURCES_NAME "${RESOURCES_DIR}" NAME)  # "CandyCrisisResources"

# --- Shared package (UI, backdrops, victory screens, music, sounds) ---
set(OTHER_ARGS)
foreach(item ${OTHER_ITEMS})
    get_filename_component(name "${item}" NAME)
    message(STATUS "  + ${name}")
    list(APPEND OTHER_ARGS "--preload" "${RESOURCES_NAME}/${name}@CandyCrisisResources/${name}")
endforeach()

list(LENGTH OTHER_ITEMS OTHER_COUNT)
message(STATUS "Packaging shared resources (${OTHER_COUNT} items) -> pkg_shared...")
execute_process(
    COMMAND "${PYTHON}" "${FILE_PACKAGER}"
        "pkg_shared.data"
        "--js-output=pkg_shared.js"
        "--quiet"
        ${OTHER_ARGS}
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)
message(STATUS "  -> pkg_shared done")
