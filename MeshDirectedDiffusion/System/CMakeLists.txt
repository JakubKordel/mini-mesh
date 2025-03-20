set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/dataHashTable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/dataReceivedCallbackTable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/gradientTable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/initMeshSystem.c
  ${CMAKE_CURRENT_SOURCE_DIR}/inputPacketsHashTable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/interestCallbackTable.c
  ${CMAKE_CURRENT_SOURCE_DIR}/interestId.c
  ${CMAKE_CURRENT_SOURCE_DIR}/neighborsTable.c
)

# Add the source files to the library target
target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(System PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
