find_package(OpenCV REQUIRED)

add_library(converter_lib converter.cpp)

target_link_libraries(converter_lib ${OpenCV_LIBS})

add_library(input_manager_lib input_manager.cpp)

target_link_libraries(input_manager_lib converter_lib)

target_include_directories(input_manager_lib PRIVATE ${CMAKE_SOURCE_DIR}/lib)