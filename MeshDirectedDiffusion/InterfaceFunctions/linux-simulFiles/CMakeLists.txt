set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/packetTransmissionLINUX-SIMUL.c
  ${CMAKE_CURRENT_SOURCE_DIR}/systemDependentHelpFunctionsLINUX.c
  ${CMAKE_CURRENT_SOURCE_DIR}/universal_semaphoreLINUX.c
  ${CMAKE_CURRENT_SOURCE_DIR}/simulServer.c
)

target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(InterfaceFunctions PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
