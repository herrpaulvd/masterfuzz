add_executable(
    DecoderStack
    main.cpp
)

target_include_directories(DecoderStack PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderStack DecoderBase)
target_link_libraries(DecoderStack Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
