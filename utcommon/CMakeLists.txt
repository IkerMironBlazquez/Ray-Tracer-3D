set(COMMON_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/common/src/vector.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/config.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/config_parser.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/scene.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/scene_parser.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/camera.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/object.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/ray_tracer.cpp"
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_scene.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_camera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_intersections.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_ray_tracer.cpp"
)

add_unit_test_target(
  TARGET_NAME utcommon
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER common
  COVERAGE_DIR coverage-common
  LIBRARY_TO_LINK common
)
