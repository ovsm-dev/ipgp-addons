SET(IPGP_GUI_USGSFEED_SOURCES
	dataloadworker.cpp
	dataloadworkerprivate.cpp
	parsingthread.cpp
	quakeevent.cpp
)

SET(IPGP_GUI_USGSFEED_MOC_HEADERS
	quakeevent.h
)

SET(IPGP_GUI_USGSFEED_MOC_HEADERS
	dataloadworker.h
	dataloadworkerprivate.h
	parsingthread.h
)

SC_SETUP_GUI_LIB_SUBDIR(IPGP_GUI_USGSFEED)
