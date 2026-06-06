add_executable(${PROJECT_NAME}.test
    tests.cpp)

target_link_libraries(${PROJECT_NAME}.test
    ${PROJECT_NAME}
    gtest_main
    gmock)

add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}.test)

install(TARGETS ${PROJECT_NAME}.test
    RUNTIME DESTINATION "${tool_dest}")