add_executable(
    DecoderWDivByZero
    main.cpp
)

target_include_directories(DecoderWDivByZero PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderWDivByZero DecoderBase)
target_link_libraries(DecoderWDivByZero Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
