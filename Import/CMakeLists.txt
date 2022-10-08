cmake_minimum_required(VERSION 3.3.0)

project(GacGen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SRCS
    Vlpp.cpp
    VlppOS.cpp
    VlppRegex.cpp
    VlppParser.cpp
    VlppGlrParser.cpp
    VlppReflection.cpp
    VlppWorkflowCompiler.cpp
    VlppWorkflowLibrary.cpp
    VlppWorkflowRuntime.cpp
    ../Release/GacUI.cpp
    ../Release/GacUICompiler.cpp
    ../Release/GacUIReflection.cpp
    ../Tools/GacGen/GacGen/GacGen.cpp
    ../Tools/GacGen/GacGen/Main.cpp
    ../Tools/GacGen/GacGen/NativeController.cpp
)

if (WIN32)
    list(APPEND SRCS Vlpp.Windows.cpp VlppOS.Windows.cpp)
else()
    list(APPEND SRCS Vlpp.Linux.cpp VlppOS.Linux.cpp)
endif()

add_executable(GacGen ${SRCS})

if(MSVC)
    target_compile_options(GacGen PRIVATE /bigobj)
    target_compile_definitions(GacGen PRIVATE UNICODE _UNICODE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(GacGen PRIVATE VCZH_DEBUG_METAONLY_REFLECTION)
endif()

target_include_directories(GacGen PRIVATE ../Tools/GacGen/GacGen ${CMAKE_CURRENT_LIST_DIR} ../Release)

install(TARGETS GacGen RUNTIME DESTINATION "bin")
