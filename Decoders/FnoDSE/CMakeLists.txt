add_executable(
    DecoderFnoDSE
    main.cpp
)

target_include_directories(DecoderFnoDSE PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderFnoDSE DecoderBase)
target_link_libraries(DecoderFnoDSE Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
