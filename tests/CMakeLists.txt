add_executable(tests dummy.cpp cpu/shifter.cpp cpu/test-decoding.cpp)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(tests
    PRIVATE 
        Catch2::Catch2WithMain
        fgba::cpu
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/deps/Catch2/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
