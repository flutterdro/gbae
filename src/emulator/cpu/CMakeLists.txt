
add_library(fgba_cpu)
add_library(fgba::cpu ALIAS fgba_cpu)

set(FGBA_CPU_SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/arm7tdmi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/opcodes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instruction-implementation/implementation.cpp
)

target_sources(fgba_cpu PUBLIC ${FGBA_CPU_SOURCES})
target_include_directories(fgba_cpu PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(fgba_cpu PUBLIC fmt::fmt)
