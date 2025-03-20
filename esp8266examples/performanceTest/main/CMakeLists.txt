idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "")
                    
# Add the new directory to COMPONENT_DIRS
list(APPEND COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/MeshDirectedDiffusion)
