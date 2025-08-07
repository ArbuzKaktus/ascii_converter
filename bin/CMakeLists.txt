add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} input_manager_lib)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)