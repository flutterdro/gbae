find_program(CPPCHECK_EXE "cppcheck")
find_program(CLANG_TIDY_EXE "clang-tidy")

if(CLANG_TIDY_EXE)
    set(CLANG_TIDY_COMMAND
        "${CLANG_TIDY_EXE}"
        "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
        -extra-arg=-Wno-unknown-warning-option
        -extra-arg=-Wno-ignored-optimization-argument
        -extra-arg=-Wno-unused-command-line-argument
        -p
    )
# set standard
    if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
        if("${CLANG_TIDY_OPTIONS_DRIVER_MODE}" STREQUAL "cl")
            set(CLANG_TIDY_COMMAND ${CLANG_TIDY_COMMAND} -extra-arg=/std:c++${CMAKE_CXX_STANDARD})
        else()
            set(CLANG_TIDY_COMMAND ${CLANG_TIDY_COMMAND} -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
        endif()
    endif()
else()
    message(AUTHOR_WARNING "Couldn't find clang-tidy")
endif()

if(CPPCHECK_EXE)
    if(CMAKE_GENERATOR MATCHES ".*Visual Studio.*")
        set(CPPCHECK_TEMPLATE "vs")
    else()
        set(CPPCHECK_TEMPLATE "gcc")
    endif()
    set(SUPPRESS_DIR "*:${CMAKE_CURRENT_BINARY_DIR}/deps/*.h")
    set(CPPCHECK_COMMAND
        ${CPPCHECK_EXE}
        --template=${CPPCHECK_TEMPLATE}
        --enable=style,performance,warning,portability
        --inline-suppr
        # We cannot act on a bug/missing feature of cppcheck
        --suppress=cppcheckError
        --suppress=internalAstError
        # if a file does not have an internalAstError, we get an unmatchedSuppression error
        --suppress=unmatchedSuppression
        # noisy and incorrect sometimes
        --suppress=passedByValue
        # ignores code that cppcheck thinks is invalid C++
        --suppress=syntaxError
        --suppress=preprocessorErrorDirective
        --inconclusive
        --suppress=${SUPPRESS_DIR})

    if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
        set(CPPCHECK_COMMAND ${CPPCHECK_COMMAND} --std=c++${CMAKE_CXX_STANDARD})
    endif()
else()
    message(AUTHOR_WARNING "Couldn't find cppcheck")
endif()

function(
    target_enable_static_analysis
        TARGET
)
    if(CPPCHECK_EXE)
        set_target_properties(${TARGET} PROPERTIES CXX_CPPCHECK "${CPPCHECK_COMMAND}")
    endif()
    if(CLANG_TIDY_EXE)
        set_target_properties(${TARGET} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
    endif()
    
endfunction()
