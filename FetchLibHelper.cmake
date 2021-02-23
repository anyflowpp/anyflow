
set(DEP_REPO http://0.2.0.2)
include(FetchContent)

macro(AddFetchLib addr name)
    FetchContent_Declare(
        ${name}
        GIT_REPOSITORY ${addr}
    )
    FetchContent_GetProperties(${name})
    if(NOT ${name}_POPULATED)    # if û�а�װ�˵������� 
        FetchContent_Populate(${name}) # ��װһ��, ���ǻ�ȡһ�¶�Ӧ�ı���
    endif()
endmacro(AddFetchLib)

function(CopyLibFileToSource name src) 
    FetchContent_GetProperties(${name})
    file(COPY ${${name}_SOURCE_DIR}/${src} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/${CORP_NAME}_${PNAME})
endfunction()