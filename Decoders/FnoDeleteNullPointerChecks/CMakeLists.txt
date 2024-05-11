add_executable(
    DecoderFnoDeleteNullPointerChecks
    main.cpp
)

target_include_directories(DecoderFnoDeleteNullPointerChecks PRIVATE "${PROJECT_SOURCE_DIR}")

target_link_libraries(DecoderFnoDeleteNullPointerChecks DecoderBase)
target_link_libraries(DecoderFnoDeleteNullPointerChecks Instances)

configure_file("${PROJECT_SOURCE_DIR}/Include/SmartPointer.h" "SmartPointer.h" COPYONLY)
