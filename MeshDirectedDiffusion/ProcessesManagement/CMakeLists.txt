set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/broadcastSelfInformationPacket.c
  ${CMAKE_CURRENT_SOURCE_DIR}/inputPacketsHandlingProcess.c
  ${CMAKE_CURRENT_SOURCE_DIR}/meshClock.c
  ${CMAKE_CURRENT_SOURCE_DIR}/packetReceivedCallback.c
  ${CMAKE_CURRENT_SOURCE_DIR}/sendInfoRequestPacket.c
  ${CMAKE_CURRENT_SOURCE_DIR}/systemManagementProcess.c
  ${CMAKE_CURRENT_SOURCE_DIR}/transmitDataProcess.c
)

target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(ProcessesManagement PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
