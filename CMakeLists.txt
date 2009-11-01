project(XtestPlugin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${XtestPlugin_SOURCE_DIR}/cmake/)

find_package(KDE4 REQUIRED)

find_package(KDevPlatform REQUIRED)
include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${KDE4_INCLUDE_DIR}/threadweaver
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(KDevVeritas REQUIRED)

include_directories(
    ${KDEVVERITAS_INCLUDE_DIR}
    )

add_subdirectory(config)
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )
add_subdirectory(qtest)
