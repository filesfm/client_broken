add_libraryFMOverlays MODULE
    DllMain.cpp
FMOverlay.cpp
FMOverlayFactory.cpp
FMOverlayRegistrationHandler.cpp
FMOverlay.rc
FMOverlays.def
)

target_link_librariesFMOverlays
FMUtil)

install(TARGETSFMOverlays
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)
