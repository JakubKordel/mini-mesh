# Define source directories for components
set(COMPONENT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/AesLibrary
    ${CMAKE_CURRENT_SOURCE_DIR}/InterfaceFunctions
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshAPI
    ${CMAKE_CURRENT_SOURCE_DIR}/PacketQueues
    ${CMAKE_CURRENT_SOURCE_DIR}/Packets
    ${CMAKE_CURRENT_SOURCE_DIR}/ProcessesManagement
    ${CMAKE_CURRENT_SOURCE_DIR}/System
)

# Include directories for headers
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/AesLibrary/include
    ${CMAKE_CURRENT_SOURCE_DIR}/InterfaceFunctions/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MeshAPI/include
    ${CMAKE_CURRENT_SOURCE_DIR}/PacketQueues/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Packets/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ProcessesManagement/include
    ${CMAKE_CURRENT_SOURCE_DIR}/System/include
    # Add mbedtls include directory
    /usr/include/mbedtls  # Adjust this path based on your mbedtls installation directory
)

# Find mbedtls library
find_library(MBEDTLS_LIBRARY mbedtls)

if(NOT MBEDTLS_LIBRARY)
    message(FATAL_ERROR "mbedtls library not found")
endif()

# Define a list to hold all source files
set(SOURCES "")

# Loop through each component directory and perform file globbing for .c files
foreach(component_dir ${COMPONENT_DIRS})
    file(GLOB component_sources "${component_dir}/*.c")
    list(APPEND SOURCES ${component_sources})
endforeach()

# Create a library target for the entire project
add_library(MeshDirectedDiffusion ${SOURCES})

# Link against mbedtls libraries and pthread
target_link_libraries(MeshDirectedDiffusion ${MBEDTLS_LIBRARY} mbedx509 mbedcrypto pthread)
