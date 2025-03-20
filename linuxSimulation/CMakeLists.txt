cmake_minimum_required(VERSION 3.10)
project(MyProject)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion)

add_executable(NodeStarter nodeStarter.c simulServer/simulServer.c simulNodes/senderNode.c simulNodes/relayingNode.c simulNodes/receiverNode.c simulNodes/mirrorNode.c simulNodes/toMirrorSenderNode.c )

add_executable(Simulation simulation.c)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/AesLibrary/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/InterfaceFunctions/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/MeshAPI/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/PacketQueues/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/Packets/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/ProcessesManagement/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/Sha256Library/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion/System/include
)

target_link_libraries(NodeStarter MeshDirectedDiffusion)

target_link_libraries(Simulation MeshDirectedDiffusion)
