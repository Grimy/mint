project(memory)

set(${PROJECT_NAME}_BUILTIN_HDR
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/builtin/array.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/builtin/hash.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/builtin/iterator.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/builtin/libobject.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/builtin/library.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/builtin/string.h
)

set(${PROJECT_NAME}_HDR
    ${${PROJECT_NAME}_BUILTIN_HDR}
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/data.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/casttool.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/class.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/garbagecollector.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/globaldata.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/memorytool.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/object.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/operatortool.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/reference.h
    ${MINT_INCLUDE_DIR}/${PROJECT_NAME}/symboltable.h
)

set(${PROJECT_NAME}_BUILTIN_SRC
    builtin/array.cpp
    builtin/hash.cpp
    builtin/iterator.cpp
    builtin/libobject.cpp
    builtin/library.cpp
    builtin/string.cpp
)

set(${PROJECT_NAME}_SRC
    ${${PROJECT_NAME}_BUILTIN_SRC}
    casttool.cpp
    class.cpp
    garbagecollector.cpp
    globaldata.cpp
    memorytool.cpp
    object.cpp
    operatortool.cpp
    reference.cpp
    symboltable.cpp
)

add_library(${PROJECT_NAME}
    OBJECT
    ${${PROJECT_NAME}_HDR}
    ${${PROJECT_NAME}_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
