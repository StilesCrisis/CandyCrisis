# Copy per-level web packages (pkg_level_*.js and .data) into the dist folder.
# Called as a POST_BUILD script: -D SRC_DIR=... -D DST_DIR=...

file(GLOB PKG_FILES "${SRC_DIR}/pkg_level_*")
foreach(F ${PKG_FILES})
    get_filename_component(NAME "${F}" NAME)
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${F}" "${DST_DIR}/${NAME}")
endforeach()
