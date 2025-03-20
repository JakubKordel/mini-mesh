# FindMeshDirectedDiffusion.cmake

# Define a module to find MeshDirectedDiffusion library
# This file should be placed in a directory where CMake can find it (e.g., /usr/share/cmake/Modules/)

# Set the name of the package
set(MeshDirectedDiffusion_NAME MeshDirectedDiffusion)

# Define variables for include directories and library targets
set(MeshDirectedDiffusion_INCLUDE_DIRS "/path/to/MeshDirectedDiffusion/include")
set(MeshDirectedDiffusion_LIBRARY MeshDirectedDiffusion)

# Add MeshDirectedDiffusion_INCLUDE_DIRS to the CMake search path
list(APPEND CMAKE_MODULE_PATH "${MeshDirectedDiffusion_INCLUDE_DIRS}")

# Provide information about MeshDirectedDiffusion
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MeshDirectedDiffusion DEFAULT_MSG
    MeshDirectedDiffusion_INCLUDE_DIRS MeshDirectedDiffusion_LIBRARY)
