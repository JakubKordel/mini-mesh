set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/packetEncryption.c
  ${CMAKE_CURRENT_SOURCE_DIR}/packetsBuilder.c
  ${CMAKE_CURRENT_SOURCE_DIR}/packetsHandler.c
)

target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(Packets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
