kdevplatform_add_plugin(kdevcargo
    JSON kdevcargo.json
    SOURCES kdevcargo.cpp
)

target_link_libraries(kdevcargo
    KDev::Interfaces
)
