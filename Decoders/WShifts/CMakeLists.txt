add_executable(
    DecoderWShifts
    main.cpp
)

target_include_directories(DecoderWShifts PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderWShifts DecoderBase)
target_link_libraries(DecoderWShifts Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
