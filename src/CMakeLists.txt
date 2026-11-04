set(SBMAN_SOURCES
    main.cpp
    securebootmanager.cpp
    securebootmanager.h)

qt_add_executable(${PROJECT_NAME}
    MANUAL_FINALIZATION
    ${SBMAN_SOURCES})

qt_add_qml_module(${PROJECT_NAME}
    URI app.nxos.sbman
    VERSION 1.0
    NO_PLUGIN
    NO_IMPORT_SCAN
    QML_FILES
        main.qml)

target_compile_options(${PROJECT_NAME} PRIVATE ${SBMAN_ARCH_FLAGS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        MauiKit4
        Qt6::Core
        Qt6::Quick
        Qt6::Qml
        KF6::CoreAddons
        KF6::I18n
        KF6::I18nQml)

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../org.nxos.sbman.desktop
    DESTINATION ${KDE_INSTALL_APPDIR})

qt_finalize_target(${PROJECT_NAME})
