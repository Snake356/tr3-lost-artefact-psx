# Fills the distribution folder from a finished build
# (run by the dist target: cmake -DDIST_CONFIG=<dist_config.cmake> -DCONFIG=<config> -P dist.cmake).
#   <dist>/<name>.exe, data.pak, tools/*.exe, the VC runtime DLLs, README.txt, CHANGES.txt, covers/, licenses/
# The settings (folders, version, runtime DLLs) come from dist_config.cmake, written at configure time.
if(NOT DEFINED DIST_CONFIG)
  message(FATAL_ERROR "dist.cmake: DIST_CONFIG not given")
endif()
include(${DIST_CONFIG})

if(CONFIG STREQUAL "Debug")
  # the debug runtime may not be passed on, and a debug build is not for players
  message(STATUS "dist: skipped for the Debug build (build Release for the distribution)")
  return()
endif()

set(bin ${BUILD_DIR}/${CONFIG})
file(REMOVE_RECURSE ${DIST_DIR})
file(MAKE_DIRECTORY ${DIST_DIR}/tools)
file(COPY "${bin}/${BUILDER_NAME}.exe" "${bin}/data.pak" DESTINATION ${DIST_DIR})
file(GLOB tools "${BUILD_DIR}/tools/${CONFIG}/*.exe")
list(LENGTH tools n)
if(NOT n EQUAL 8)
  message(FATAL_ERROR "dist: expected the 8 conversion programs in ${BUILD_DIR}/tools/${CONFIG}, found ${n}")
endif()
file(COPY ${tools} DESTINATION ${DIST_DIR}/tools)
if(NOT RUNTIME_DLLS)
  message(FATAL_ERROR "dist: the Visual C++ runtime DLLs were not found (InstallRequiredSystemLibraries)")
endif()
# next to the EXE and next to the conversion programs: Windows looks for a program's DLLs in its own folder,
# not in the folder of the program that started it. The conversion programs are C and need only vcruntime.
file(COPY ${RUNTIME_DLLS} DESTINATION ${DIST_DIR})
set(c_runtime ${RUNTIME_DLLS})
list(FILTER c_runtime INCLUDE REGEX "/vcruntime[^/]*\\.dll$")
file(COPY ${c_runtime} DESTINATION ${DIST_DIR}/tools)
# the players' readme and list of changes, the covers for emulator frontends (as the builder puts one next to
# the disc image) and the licence of the conversion programs
file(COPY "${PACKAGE_DIR}/README.txt" "${PACKAGE_DIR}/CHANGES.txt" DESTINATION ${DIST_DIR})
file(COPY "${PACKAGE_DIR}/licenses/GPL-3.0.txt" DESTINATION ${DIST_DIR}/licenses)
foreach(cover pal:PAL usa:USA japan:Japan)
  string(REPLACE ":" ";" cover ${cover})
  list(GET cover 0 file)
  list(GET cover 1 title)
  configure_file("${COVERS_DIR}/${file}.png" "${DIST_DIR}/covers/Tomb Raider III - The Lost Artefact (${title}).png" COPYONLY)
endforeach()
message(STATUS "dist: ${DIST_DIR} (version ${BUILDER_VERSION_FULL})")
