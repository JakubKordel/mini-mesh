set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/PacketsBuffer.c
)

target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(PacketQueues PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
