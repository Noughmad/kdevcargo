add_definitions(-DTRANSLATION_DOMAIN=\"kdevcargo\")

ecm_qt_declare_logging_category(cargo_LOG_SRCS
    HEADER debug.h
    IDENTIFIER CUSTOMBUILDSYSTEM
    CATEGORY_NAME "kdevelop.projectmanagers.cargo"
)
## KDevelop Plugin
set(cargo_SRCS
    cargoplugin.cpp
    cargobuildjob.cpp
    cargoexecutionconfig.cpp
    ${cargo_LOG_SRCS}
)

ki18n_wrap_ui( cargo_SRCS cargoexecutionconfig.ui )
kdevplatform_add_plugin(kdevcargo JSON kdevcargo.json SOURCES ${cargo_SRCS})
target_link_libraries(kdevcargo
      KDev::Project
      KDev::Language
      KDev::Interfaces
      KDev::Util
      KDev::OutputView
)

## Unittests
add_subdirectory( tests )
