SET(GUI_CORE_SOURCES
    databasemanager.cpp
    pdfbulletin.cpp
)

SET(GUI_CORE_MOC_HEADERS
    databasemanager.h
    pdfbulletin.h
)

SC_SETUP_GUI_LIB_SUBDIR(GUI_CORE)
