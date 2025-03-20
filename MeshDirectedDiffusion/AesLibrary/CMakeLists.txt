set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/aes.c
)

target_sources(MeshDirectedDiffusion PRIVATE ${SOURCES})

target_include_directories(AesLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
