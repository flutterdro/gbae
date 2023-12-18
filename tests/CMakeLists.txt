add_executable(tests dummy.cpp)

target_link_libraries(tests
    PRIVATE 
        Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/third_party/Catch2/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)