add_executable(
    DecoderFnobuiltin
    main.cpp
)

target_include_directories(DecoderFnobuiltin PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderFnobuiltin DecoderBase)
target_link_libraries(DecoderFnobuiltin Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
