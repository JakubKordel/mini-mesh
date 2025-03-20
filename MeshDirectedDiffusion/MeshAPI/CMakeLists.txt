set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/meshAPI.c
)

target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(MeshAPI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
