add_library(common STATIC)

target_sources(common 
    PRIVATE 
        src/vector.cpp
        src/config.cpp
        src/config_parser.cpp
        src/scene.cpp
        src/scene_parser.cpp
        src/camera.cpp
        src/object.cpp
        src/ray_tracer.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(common PUBLIC Microsoft.GSL::GSL TBB::tbb)