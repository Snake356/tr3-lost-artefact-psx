cmake_minimum_required(VERSION 3.25)
project(TR3LostArtefactPS1Builder VERSION 1.12.0 LANGUAGES C CXX)   # the one place of the version (EXE, manifest, ZIP)
set(BUILDER_NAME "TR3 Lost Artefact PS1 Builder")
set(BUILDER_VERSION_TWEAK 0)
set(BUILDER_VERSION_FULL "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${BUILDER_VERSION_TWEAK}")

# the C runtime as DLL (/MD) for the builder and the eight conversion programs: one shared runtime instead of
# nine static copies, serviced like any VC++ program; the DLLs are shipped in dist\ (app-local, no installer needed)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(TR3LA_PROJECT_DIR "${CMAKE_SOURCE_DIR}/../.." CACHE PATH "the Lost Artefact PS1 project (data the builder carries)")
if(NOT EXISTS "${TR3LA_PROJECT_DIR}/levels/convert.sh")
  message(FATAL_ERROR "TR3LA_PROJECT_DIR does not point to the Lost Artefact PS1 project: ${TR3LA_PROJECT_DIR}")
endif()

add_subdirectory(external)

# the data pack: the project files the builder reads at run time, shipped as data.pak next to the EXE
add_executable(packdata packer/packdata.cpp)
set_target_properties(packdata PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
target_compile_options(packdata PRIVATE /utf-8 /EHsc)
set(PAK_DIR ${CMAKE_BINARY_DIR}/pak)
set(TOOL_TARGETS tool_tr3pcpsx tool_trmod tool_troi tool_trsfx tool_tr3mesh tool_texmod tool_3texex tool_3texrep)
add_custom_target(datapak ALL
    COMMAND packdata "${TR3LA_PROJECT_DIR}" "${PAK_DIR}/data.pak"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TR3LA_PROJECT_DIR}/tools/builder/icon.ico" "${PAK_DIR}/icon.ico"
    BYPRODUCTS ${PAK_DIR}/data.pak ${PAK_DIR}/icon.ico
    DEPENDS packdata
    COMMENT "Packing the builder's data")

file(GLOB_RECURSE BUILDER_SRC CONFIGURE_DEPENDS src/*.cpp src/*.h)
# the level tools are in groups (src/leveltools/<group>/); a build can take only some of them
set(LEVELTOOL_GROUPS "all" CACHE STRING "level tool groups to build (folders of src/leveltools), or all")
if(NOT LEVELTOOL_GROUPS STREQUAL "all")
  list(FILTER BUILDER_SRC EXCLUDE REGEX "/src/leveltools/[^/]+/")
  foreach(group IN LISTS LEVELTOOL_GROUPS)
    file(GLOB_RECURSE GROUP_SRC CONFIGURE_DEPENDS src/leveltools/${group}/*.cpp src/leveltools/${group}/*.h)
    list(APPEND BUILDER_SRC ${GROUP_SRC})
  endforeach()
endif()
# the resources: icon, manifest and VERSIONINFO, both generated from project(VERSION)
set(GEN_DIR ${CMAKE_BINARY_DIR}/generated)
configure_file(src/app/version.h.in ${GEN_DIR}/version.h @ONLY)
configure_file(src/app/builder.manifest.in ${GEN_DIR}/builder.manifest @ONLY)
add_executable(builder WIN32 ${BUILDER_SRC} src/app/builder.rc)
set_source_files_properties(src/app/builder.rc PROPERTIES OBJECT_DEPENDS "${PAK_DIR}/icon.ico;${GEN_DIR}/builder.manifest;${GEN_DIR}/version.h")
set_target_properties(builder PROPERTIES OUTPUT_NAME "${BUILDER_NAME}" CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
target_include_directories(builder PRIVATE ${PAK_DIR} ${GEN_DIR})
target_compile_definitions(builder PRIVATE UNICODE _UNICODE NOMINMAX _CRT_SECURE_NO_WARNINGS)
target_compile_options(builder PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4 /utf-8 /permissive- /Zc:__cplusplus /MP /EHsc>)
target_link_options(builder PRIVATE /MANIFEST:NO)
target_link_libraries(builder PRIVATE comctl32 shell32 ole32 uxtheme dwmapi gdiplus)
add_dependencies(builder datapak ${TOOL_TARGETS})

# the builder runs from its build folder as from the distribution: data.pak and tools\ next to the EXE
add_custom_command(TARGET builder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PAK_DIR}/data.pak $<TARGET_FILE_DIR:builder>/data.pak
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/tools/$<CONFIG> $<TARGET_FILE_DIR:builder>/tools)

# the distribution folder <build>/dist: EXE, data.pak, tools\, the VC runtime DLLs, README.txt (dist.ps1 zips it)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)      # only find them, no install() rules
include(InstallRequiredSystemLibraries)
set(DIST_DIR ${CMAKE_BINARY_DIR}/dist)
file(CONFIGURE OUTPUT ${GEN_DIR}/dist_config.cmake @ONLY CONTENT [[
set(BUILD_DIR "@CMAKE_BINARY_DIR@")
set(DIST_DIR "@DIST_DIR@")
set(BUILDER_NAME "@BUILDER_NAME@")
set(BUILDER_VERSION_FULL "@BUILDER_VERSION_FULL@")
set(PROJECT_VERSION "@PROJECT_VERSION@")
set(RUNTIME_DLLS "@CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS@")
set(PACKAGE_DIR "@TR3LA_PROJECT_DIR@/tools/builder/package")
set(COVERS_DIR "@TR3LA_PROJECT_DIR@/tools/builder/covers")
]])
add_custom_target(dist ALL
    COMMAND ${CMAKE_COMMAND} -DDIST_CONFIG=${GEN_DIR}/dist_config.cmake -DCONFIG=$<CONFIG> -P ${CMAKE_SOURCE_DIR}/cmake/dist.cmake
    DEPENDS builder
    COMMENT "Filling the distribution folder")
