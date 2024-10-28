set(PROJECT_NAME WfpExample)

get_source_files(src ALL_FILES)
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
