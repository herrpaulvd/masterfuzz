add_executable(
    SimpleFuzzer
    main.cpp
)

target_include_directories(SimpleFuzzer PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(SimpleFuzzer DecoderBase)
target_link_libraries(SimpleFuzzer Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
configure_file("${PROJECT_SOURCE_DIR}/Include/Common.h" "Common.h" COPYONLY)
