add_library(Instances
    Impl.cpp
)

target_include_directories(Instances PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(Instances DecoderBase)
