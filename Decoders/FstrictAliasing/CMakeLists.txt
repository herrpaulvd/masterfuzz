add_executable(
    DecoderFstrictAliasing
    main.cpp
)

target_include_directories(DecoderFstrictAliasing PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderFstrictAliasing DecoderBase)
target_link_libraries(DecoderFstrictAliasing Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
