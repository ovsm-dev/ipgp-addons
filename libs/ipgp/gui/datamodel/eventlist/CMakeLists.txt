SET(IPGP_GUI_EVENTLIST_SOURCES
    eventlistwidget.cpp
	eventlistdelegate.cpp
)

SET(IPGP_GUI_EVENTLIST_MOC_HEADERS
	eventlistwidget.h
	eventlistdelegate.h
)

SET(IPGP_GUI_EVENTLIST_UI eventlistwidget.ui)
SET(IPGP_GUI_EVENTLIST_RESOURCES eventlistwidget.qrc)

SC_SETUP_GUI_LIB_SUBDIR(IPGP_GUI_EVENTLIST)
