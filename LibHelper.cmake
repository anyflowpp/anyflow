macro(AddSharedLib libname srcdir incdir)
    file(GLOB ${libname}_src ${srcdir}/*.cpp ${srcdir}/*.c)
    file(GLOB ${libname}_head ${incdir}/*.hpp ${incdir}/*.h)
    add_library(${libname} SHARED ${${libname}_src} ${${libname}_head})
    set_target_properties(${libname} PROPERTIES COMPILE_FLAGS -D_EXPORT_API_)
    add_custom_command(TARGET ${libname} 
                                POST_BUILD
                                COMMAND ${CMAKE_SOURCE_DIR}/scripts/post-build${script_ext}
                                        ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${libname}${dll_ext}
                                        ${CMAKE_SOURCE_DIR}/${CORP_NAME}_${PNAME}
                                VERBATIM)
endmacro(AddSharedLib)