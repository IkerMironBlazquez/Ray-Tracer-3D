add_executable(render-par)
target_sources(render-par 
    PRIVATE 
      src/main.cpp
      src/image_par.cpp  # PAR: Añadir implementación de imagen PAR
)
target_include_directories(render-par PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# USE_SIMPLE_PARTITIONER por defecto
target_compile_definitions(render-par PRIVATE USE_SIMPLE_PARTITIONER)

target_link_libraries(render-par PRIVATE Microsoft.GSL::GSL common)

