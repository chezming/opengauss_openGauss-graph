#This is the main CMAKE for build bin.
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} TGT_orc_SRC)

set(TGT_orc_INC 
    ${PROJECT_SRC_DIR}/gausskernel/cbb/communication
    ${PROJECT_SRC_DIR}/include/iprange
    ${PROJECT_SRC_DIR}/include/libcomm
    ${PROJECT_SRC_DIR}/include
    ${PROJECT_SRC_DIR}/lib/gstrace
    ${LIBCGROUP_INCLUDE_PATH}
    ${PROJECT_SRC_DIR}/include/libcomm
if(NOT "${ENABLE_LITE_MODE}" STREQUAL "ON")    
    ${LIBORC_INCLUDE_PATH}
    ${LIBPARQUET_INCLUDE_PATH}
endif()    
    ${PROTOBUF_INCLUDE_PATH}
    ${ZLIB_INCLUDE_PATH}
    ${LIBCURL_INCLUDE_PATH} 
)

set(orc_DEF_OPTIONS ${MACRO_OPTIONS})
set(orc_COMPILE_OPTIONS ${OPTIMIZE_OPTIONS} ${OS_OPTIONS} ${PROTECT_OPTIONS} ${WARNING_OPTIONS} ${BIN_SECURE_OPTIONS} ${CHECK_OPTIONS})
set(orc_LINK_OPTIONS ${BIN_LINK_OPTIONS})
add_static_objtarget(gausskernel_storage_access_dfs_orc TGT_orc_SRC TGT_orc_INC "${orc_DEF_OPTIONS}" "${orc_COMPILE_OPTIONS}" "${orc_LINK_OPTIONS}")

