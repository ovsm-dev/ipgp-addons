SET(GUI_CLIENT_SOURCES
		application.cpp
		messagereader.cpp
		scheme.cpp
)

SET(GUI_CLIENT_HEADERS
		misc.h
		scheme.h
)

SET(GUI_CLIENT_MOC_HEADERS
		application.h
		messagereader.h
#		misc.h
#		scheme.h
)

SET(GUI_CLIENT_UI
	    about.ui
		connectionsetup.ui
		plugindialog.ui
)

SET(GUI_CLIENT_RESOURCES client.qrc)

SC_SETUP_GUI_LIB_SUBDIR(GUI_CLIENT)
