project(plasma-tide)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)


set(tide_QML
    main.qml
)


set(tide_metadata
    metadata.desktop
)

add_custom_target(QmlFiles ALL ${CMAKE_COMMAND} -E echo "main.qml" SOURCES ${tide_QML})

install(FILES ${tide_QML}
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/com.github.plasma-tide/contents/ui)

install(FILES ${tide_metadata}
	DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/com.github.plasma-tide/contents)

install(FILES ${tide_metadata}
	DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-plasma-tide.desktop)

