add_executable(
    DecoderFwrapv
    main.cpp
)

target_include_directories(DecoderFwrapv PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderFwrapv DecoderBase)
target_link_libraries(DecoderFwrapv Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
