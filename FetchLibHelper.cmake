
set(DEP_REPO http://0.2.0.2)
include(FetchContent)

macro(AddFetchLib addr name)
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