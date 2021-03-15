if(UNIX)
cmake_minimum_required(VERSION 3.16)
else()
cmake_minimum_required(VERSION 3.18)
endif()

set(PNAME anyflow)
set(CORP_NAME Demo)
project(${PNAME})
set(CMAKE_CXX_STANDARD 14)
execute_process(COMMAND git rev-parse --short HEAD
                TIMEOUT 5
                OUTPUT_VARIABLE GIT_COMMIT_ID
                OUTPUT_STRIP_TRAILING_WHITESPACE
                )               
add_definitions(-DHEAD_COMMIT_ID=${GIT_COMMIT_ID})
include_directories(${CMAKE_SOURCE_DIR}/src)

if(UNIX)
    set(exe_ext "")
    set(script_ext ".sh")
    set(dll_ext ".so")
else()
    set(exe_ext ".exe")
    set(script_ext ".bat")
    set(dll_ext ".dll")
endif()

option(DBG "build debug" OFF)
if(DBG)
    set(CMAE_CXX_FLAGS "-O0 -ggdb")
endif()
set(CMAE_CXX_FLAGS_DEBUG "-O0 -ggdb")

include(FetchLibHelper.cmake)

AddFetchLib(spdlog-1.5.0)

# src file config
file(GLOB src src/*.cpp src/*.c src/core/*.cpp)
file(GLOB head src/*.h src/*.hpp src/core/*.h)

# json config 
set(JSON_DIR ${CMAKE_SOURCE_DIR}/3rdPart/jsoncpp)
file(GLOB json_src ${JSON_DIR}/*cpp)
file(GLOB json_head ${JSON_DIR}/json/*.h)
source_group("JSON" FILES ${json_head} ${json_src})
include_directories(${JSON_DIR})

# base64 config 
set(B64_DIR ${CMAKE_SOURCE_DIR}/3rdPart/base64)
file(GLOB base64_src ${B64_DIR}/src/*cpp)
file(GLOB base64_head ${B64_DIR}/include/*.h)
source_group("base64" FILES ${base64_head} ${base64_src})
include_directories(${B64_DIR}/include)

include(LibHelper.cmake)

option(BuildTest "build test" ON)
if (BuildTest)
    set(TestTool "TestTool")
    AddFetchLib(googletest)
    FetchContent_GetProperties(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

    file(GLOB test_src test/*.cpp test/*.c)
    file(GLOB test_head test/*.h test/*.hpp)
    add_executable(${TestTool} ${test_src} ${test_head})
    target_include_directories(${TestTool} PRIVATE ${googletest_SOURCE_DIR}/include 
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(UNIX)
        target_link_libraries(${TestTool} gtest_main dl)
    else()
        target_link_libraries(${TestTool} gtest_main)
    endif()
    set(pro_dir ${CMAKE_SOURCE_DIR}/build)
    add_custom_command(TARGET ${TestTool} 
                             POST_BUILD
                             COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/post-build${script_ext}
                                      ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/TestTool${exe_ext}
                                      ${CMAKE_CURRENT_SOURCE_DIR}/${CORP_NAME}_${PNAME}
                             VERBATIM )
    set_target_properties(${TestTool} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${CORP_NAME}_${PNAME}")
    set_target_properties(${TestTool} PROPERTIES VS_DEBUGGER_COMMAND "${CMAKE_SOURCE_DIR}/${CORP_NAME}_${PNAME}/TestTool${exe_ext}")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TestTool})
endif()