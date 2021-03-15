
set(DEP_REPO http://www.lib_repo.com)
include(FetchContent)
macro(AddFetchLib name)
    FetchContent_Declare(
        ${name}
        URL   ${DEP_REPO}/${name}.7z
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)    # if 没有安装了第三方库 
        FetchContent_Populate(${name}) # 安装一下, 就是获取一下对应的变量
    endif()
    include(${${name}_SOURCE_DIR}/Config.cmake)
endmacro(AddFetchLib)

macro(AddFetchLib_GIT addr name)
    FetchContent_Declare(
        ${name}
        GIT_REPOSITORY ${addr}
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)    # if 没有安装了第三方库 
        FetchContent_Populate(${name}) # 安装一下, 就是获取一下对应的变量
    endif()
endmacro(AddFetchLib)

function(CopyLibFileToSource name src) 
    FetchContent_GetProperties(${name})
    file(COPY ${${name}_SOURCE_DIR}/${src} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/${CORP_NAME}_${PNAME})
endfunction()