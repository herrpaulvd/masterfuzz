add_executable(
    DecoderUB
    main.cpp
)

target_include_directories(DecoderUB PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderUB DecoderBase)
target_link_libraries(DecoderUB Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
