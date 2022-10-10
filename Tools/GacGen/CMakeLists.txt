cmake_minimum_required(VERSION 3.3.0)

project(GacGen LANGUAGES CXX)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TARGET_ARCH 32)
else()
    set(TARGET_ARCH 64)
endif()

set(CMAKE_CXX_STANDARD 20)

set(SRCS
    ../../Import/Vlpp.cpp
    ../../Import/VlppOS.cpp
    ../../Import/VlppRegex.cpp
    ../../Import/VlppParser.cpp
    ../../Import/VlppGlrParser.cpp
    ../../Import/VlppReflection.cpp
    ../../Import/VlppWorkflowCompiler.cpp
    ../../Import/VlppWorkflowLibrary.cpp
    ../../Import/VlppWorkflowRuntime.cpp
    ../../Release/GacUI.cpp
    ../../Release/GacUICompiler.cpp
    ../../Release/GacUIReflection.cpp
    GacGen/GacGen.cpp
    GacGen/Main.cpp
    GacGen/NativeController.cpp
)

if (WIN32)
    list(APPEND SRCS ../../Import/Vlpp.Windows.cpp ../../Import/VlppOS.Windows.cpp)
else()
    list(APPEND SRCS ../../Import/Vlpp.Linux.cpp ../../Import/VlppOS.Linux.cpp)
endif()

add_executable(GacGen ${SRCS})

if(MSVC)
    target_compile_options(GacGen PRIVATE /bigobj)
    target_compile_definitions(GacGen PRIVATE UNICODE _UNICODE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(GacGen PRIVATE VCZH_DEBUG_METAONLY_REFLECTION)
endif()

target_include_directories(GacGen PRIVATE GacGen ${CMAKE_CURRENT_LIST_DIR} ../../Release)

set_target_properties(GacGen PROPERTIES RUNTIME_OUTPUT_NAME GacGen${TARGET_ARCH})

install(TARGETS GacGen RUNTIME DESTINATION "bin")
