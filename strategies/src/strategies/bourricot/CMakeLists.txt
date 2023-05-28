set(LIB_NAME bourricot)

set(STRATEGY_CXX
#    bourricot.cpp
#    bourricot...analyser.cpp)
)

set(LIBRARY_OUTPUT_PATH ../../../strategies)

add_library(${LIB_NAME} SHARED
    ${STRATEGY_CXX})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(${LIB_NAME}
        siis
        pthread
        rt
        ${TA_LIBRARIES})
endif()

install (TARGETS ${LIB_NAME}
    LIBRARY DESTINATION strategies
    ARCHIVE DESTINATION strategies
    RUNTIME DESTINATION strategies
    COMPONENT library)
