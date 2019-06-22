set(LIB_NAME simpleml)

set(SUPERVISOR_CXX
    simpleml.cpp)

set(LIBRARY_OUTPUT_PATH ../../../supervisors)

add_library(${LIB_NAME} SHARED
    ${SUPERVISOR_CXX})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${LIB_NAME}
        siis
        pthread
        rt
        ${TA_LIBRARIES})
endif()

install (TARGETS ${LIB_NAME}
    LIBRARY DESTINATION supervisors
    ARCHIVE DESTINATION supervisors
    RUNTIME DESTINATION supervisors
    COMPONENT library)
