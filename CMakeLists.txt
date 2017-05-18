cmake_minimum_required(VERSION 2.8.12)

project(kdevcargo)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMInstallIcons)

find_package(KDevPlatform 5.0 REQUIRED)
find_package(KF5 5.15.0 REQUIRED COMPONENTS ItemModels)

add_subdirectory(src)
add_subdirectory(icons)
