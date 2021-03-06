project(qtest)
add_subdirectory(tests)

add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9003 )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kdevqtest_SRCS
    qtestoutputparser.cpp
    qtestmodelitems.cpp
    xmlregister.cpp
    qtestoutputmorpher.cpp
    modelbuilder.cpp
    suitebuilder.cpp
    executable.cpp
    casebuilder.cpp
    qtestsettings.cpp
    qtestoutputmodel.cpp
    qtestoutputdelegate.cpp
    qtestoutputjob.cpp
    createqtestwizard.cpp)

kde4_add_ui_files(kdevqtest_SRCS
    classdetailspage.ui)

qt4_automoc(${kdevqtest_SRCS})
kde4_add_library(kdevqtest SHARED ${kdevqtest_SRCS})
target_link_libraries(kdevqtest
    ${KDEVVERITAS_LIBRARY}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_UTIL_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${QT_QTXML_LIBRARY})
install(TARGETS kdevqtest ${INSTALL_TARGETS_DEFAULT_ARGS} )

set(kdevqtestview_SRCS qtestplugin.cpp)
kde4_add_kcfg_files(kdevqtestview_SRCS qtestconfig.kcfgc)
qt4_automoc(${kdevqtestview_SRCS})
kde4_add_plugin(kdevqtestview ${kdevqtestview_SRCS})
target_link_libraries(kdevqtestview
    kdevqtest
    ${QT_QTCORE_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVVERITAS_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBS})

install(TARGETS kdevqtestview DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kdevqtest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kdevqtest.rc DESTINATION ${DATA_INSTALL_DIR}/kdevqtest )

