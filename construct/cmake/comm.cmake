include(${CMAKE_SOURCE_DIR}/construct/cmake/riscv32_toolchain.cmake)
include(${CMAKE_SOURCE_DIR}/construct/cmake/global_var.cmake)
include(${CMAKE_SOURCE_DIR}/construct/cmake/module.cmake)

# ==================================== Functions ===============================
# Function to add a test executable
function(add_gtest_executable test_name)
    # Parse arguments
    set(sources ${ARGN})
    
    # Create executable
    add_executable(${test_name} ${sources})
    
    # Link libraries
    target_link_libraries(${test_name}
        GTest::gtest
        GTest::gmock
        pthread
    )
    
    # Add to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set properties
    set_target_properties(${test_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endfunction()

# Function to add a test with custom setup
function(add_osz_test test_name)
    # Parse arguments
    set(sources ${ARGN})
    
    # Check if we have sources
    if(NOT sources)
        message(WARNING "No sources provided for test ${test_name}")
        return()
    endif()
    
    # Add the test executable
    add_gtest_executable(${test_name} ${sources})
    
    # Additional OSZ-specific include directories can be added here
    # target_include_directories(${test_name} PRIVATE
    #     ../../kernel/comm/include
    #     ../../kernel/base/include
    # )
endfunction()

# Example usage (commented out):
# add_osz_test(test_example
#     test_example.cpp
#     test_helper.cpp
# )
# ==============================================================================
