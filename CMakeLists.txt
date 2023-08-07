cmake_minimum_required(VERSION 3.15)
project(resparked VERSION 0.1.0 LANGUAGES CXX C)

# add modules to the cmake module path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# --------------------------------------
#
# Build options and settings
#
# --------------------------------------

# Set C++ standard and compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -Wpedantic)

#set(CMAKE_CXX_CLANG_TIDY clang-tidy
#    "-checks=bugprone-*,modernize-*,core-*,performance-*,readability-*,clang-analyzer-*,portability-*,-cppcoreguidelines-avoid-magic-numbers,-readability-magic-numbers"
#    "-fix"
#    "-header-filter=.*/include/.*"
#)
#set(CMAKE_CXX_CPPCHECK cppcheck "--enable=all" 
#    "--suppress=missingIncludeSystem" 
#    "--suppress=*:entt.hpp:*"
#     # skip entt.hpp
#) # You can customize the checks here

# Set up the project
set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ENGINE_SOURCES ${ENGINE_DIR}/engine.cpp)
set(PROJECT_HEADERS ${INCLUDE_DIR}/resparked.hpp)

set(LIB_NAME ${PROJECT_NAME}_impl)
set(LIB_SOURCES ${SOURCE_DIR}/resparked.cpp)

# --------------------------------------
#
# GLAD
#
# --------------------------------------

message("Configuring GLAD")
set(GLAD_VERSION 0.1.34)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(GLAD_DIR ${VENDOR_DIR}/glad)

set(GLAD_SRC
    ${GLAD_DIR}/src/glad.c
)
set(GLAD_HEADERS
    ${GLAD_DIR}/include/glad/glad.h
    ${GLAD_DIR}/include/KHR/khrplatform.h
)

list(APPEND PROJECT_HEADERS ${GLAD_HEADERS})

add_library(glad STATIC ${GLAD_SRC} ${GLAD_HEADERS})
target_include_directories(glad PUBLIC ${GLAD_DIR}/include/glad)

target_compile_definitions(glad PRIVATE GLAD_GLAPI_EXPORT_BUILD)
target_compile_definitions(glad PUBLIC GLAD_GLAPI_EXPORT)

set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

# suppress warnings for glad

set_target_properties(glad PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(glad PROPERTIES CXX_CPPCHECK "")

# --------------------------------------
#
# GLFW3 & OpenGL
#
# --------------------------------------

message("Configuring GLFW3 and OpenGL")
message("Will now attempt to find the required packages based on your system")
message("Procedure is not tested on NT (Windows) and Apple systems.")

# now onto the libraries
find_package(OpenGL REQUIRED)

if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
else(NOT OPENGL_FOUND)
    message(STATUS " OPENGL found at: ${OPENGL_INCLUDE_DIR} ${OPENGL_LIBRARIES}")
endif(NOT OPENGL_FOUND)

set(LIBS glad ${OPENGL_LIBRARIES})

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)

    if(NOT COCOA_LIBRARY)
        message(ERROR " COCOA not found!")
    else(NOT COCOA_LIBRARY)
        message(STATUS " COCOA found at: ${COCOA_LIBRARY}")
    endif(NOT COCOA_LIBRARY)

    find_library(IOKIT_LIBRARY IOKit REQUIRED)

    if(NOT IOKIT_LIBRARY)
        message(ERROR " IOKIT not found!")
    else(NOT IOKIT_LIBRARY)
        message(STATUS " IOKIT found at: ${IOKIT_LIBRARY}")
    endif(NOT IOKIT_LIBRARY)

    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    if(NOT COREVIDEO_LIBRARY)
        message(ERROR " COREVIDEO not found!")
    else(NOT COREVIDEO_LIBRARY)
        message(STATUS " COREVIDEO found at: ${COREVIDEO_LIBRARY}")
    endif(NOT COREVIDEO_LIBRARY)

    if(NOT SANDBOX_PACKAGES_NOT_FOUND)
        list(APPEND LIBS ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
    endif(NOT SANDBOX_PACKAGES_NOT_FOUND)
endif(APPLE)

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)

    if(NOT X11_FOUND)
        message(ERROR " X11 not found!")
    else(NOT X11_FOUND)
        message(STATUS " X11 found at: ${X11_INCLUDE_DIR} ${X11_LIBRARIES}")
    endif(NOT X11_FOUND)

    if(NOT CMAKE_DL_LIBS)
        message(ERROR " DL not found!")
    else(NOT CMAKE_DL_LIBS)
        message(STATUS " DL found at: ${CMAKE_DL_LIBS}")
    endif(NOT CMAKE_DL_LIBS)

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED) # find out what flag is set if Threads is found.

    find_package(XRANDR REQUIRED)

    if(NOT XRANDR_LIBRARY)
        message(ERROR " XRANDR not found!")
    else(NOT XRANDR_LIBRARY)
        message(STATUS " XRANDR found at: ${XRANDR_LIBRARY}")
    endif(NOT XRANDR_LIBRARY)

    list(APPEND LIBS ${X11_LIBRARIES} ${XRANDR_LIBRARY} ${CMAKE_DL_LIBS} Threads::Threads)
endif(UNIX AND NOT APPLE)

# windows just needs the opengl32 library, which should have already been fetched.
find_package(GLFW3 REQUIRED)

if(NOT GLFW3_FOUND)
    message(ERROR " GLFW3 not found!")
else(NOT GLFW3_FOUND)
    message(STATUS " GLFW3 found at: ${GLFW3_INCLUDE_DIR} ${GLFW3_LIBRARY}")
    list(APPEND LIBS ${GLFW3_LIBRARY})
endif(NOT GLFW3_FOUND)

# --------------------------------------
#
# Asset Importer Library
#
# --------------------------------------

message("Configuring Assimp")
find_package(assimp REQUIRED)

list(APPEND LIBS ${assimp_LIBRARIES})
list(APPEND PROJECT_HEADERS ${assimp_INCLUDE_DIRS})

# --------------------------------------
#
# ImGUI
#
# --------------------------------------

set(IMGUI_VERSION 1.89.7)
set(IMGUI_DIR ${VENDOR_DIR}/imgui-${IMGUI_VERSION})

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_HEADERS
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.h
)

add_library(imgui STATIC ${IMGUI_SRC} ${IMGUI_HEADERS})

list(APPEND PROJECT_HEADERS ${IMGUI_HEADERS})

target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_include_directories(imgui PRIVATE ${IMGUI_DIR}/backends)
target_include_directories(imgui PRIVATE ${IMGUI_DIR}/misc/cpp)

target_link_libraries(imgui PUBLIC ${LIBS})

set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)


# --------------------------------------
#
# Loguru
#
# --------------------------------------

set(LOGURU_VERSION 2.1.0)

set(LOGURU_DIR ${VENDOR_DIR}/loguru-${LOGURU_VERSION})

set(LOGURU_SRC
    ${LOGURU_DIR}/loguru.cpp
)

set(LOGURU_HEADERS
    ${LOGURU_DIR}/loguru.hpp
)

list(APPEND PROJECT_HEADERS ${LOGURU_HEADERS})
list(APPEND LIBS loguru)

add_library(loguru STATIC ${LOGURU_SRC} ${LOGURU_HEADERS})
target_include_directories(loguru PUBLIC ${LOGURU_DIR})


set_target_properties(loguru PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --------------------------------------
#
# Implementation library
#
# --------------------------------------

# create our implementation library
add_library(${LIB_NAME} SHARED ${LIB_SOURCES} ${PROJECT_HEADERS})
target_include_directories(${LIB_NAME} PRIVATE ${INCLUDE_DIR} ${GLAD_DIR}/include ${GLFW3_INCLUDE_DIR} 
${IMGUI_DIR} ${IMGUI_DIR}/backends ${IMGUI_DIR}/misc/cpp ${LOGURU_DIR})

# ----------- Link Libraries -----------

target_link_libraries(${LIB_NAME} PRIVATE ${LIBS} imgui loguru ${LIBS})
set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --------------------------------------
#
# Engine executable
#
# --------------------------------------

# Create the executable and link with the shared library
add_executable(engine ${ENGINE_SOURCES} ${PROJECT_HEADERS})
target_include_directories(engine PRIVATE ${INCLUDE_DIR})


# ----------- Link Libraries -----------

target_link_libraries(engine PRIVATE ${LIBS} ${LIB_NAME})

# --------------------------------------
#
# Custom targets
#
# --------------------------------------

add_custom_target(cppcheck
    COMMAND ${CMAKE_CXX_CPPCHECK} ${ENGINE_SOURCES} ${PROJECT_HEADERS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT " Running static analysis with cppcheck "
    VERBATIM)

# --------------------------------------
#
# Doxygen
#
# --------------------------------------
option(BUILD_DOC " Build documentation " OFF)

if(BUILD_DOC)
    # check if Doxygen is installed
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        # set input and output files
        set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        message(" Doxygen build started using ${DOXYFILE} ")

        # Copy the Doxyfile and the doxygen-custom folder to the build directory

        add_custom_target(doxygen-custom 
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-custom
            ${CMAKE_CURRENT_BINARY_DIR}/doxygen-custom
            COMMENT " Copying doxygen-custom to build directory "
            VERBATIM)

        add_custom_target(doxygen-awesome-css 
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-awesome.css
            ${CMAKE_CURRENT_BINARY_DIR}/doxygen-awesome.css
            COMMENT " Copying doxygen-awesome.css to build directory "
            VERBATIM)

        add_custom_target(Doxyfile 
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            COMMENT " Copying Doxyfile to build directory "
            VERBATIM)

        add_custom_target(build_docs 
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT " Generating API documentation with Doxygen "
            VERBATIM)

        add_dependencies(build_docs 
        Doxyfile 
        doxygen-custom 
        doxygen-awesome-css)
    else(DOXYGEN_FOUND)
        message(" Doxygen need to be installed to generate the doxygen documentation ")
    endif(DOXYGEN_FOUND)
endif(BUILD_DOC)
