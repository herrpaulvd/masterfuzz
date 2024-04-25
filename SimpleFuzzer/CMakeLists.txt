add_executable(
    SimpleFuzzer
    main.cpp
)

target_include_directories(SimpleFuzzer PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(SimpleFuzzer DecoderBase)
