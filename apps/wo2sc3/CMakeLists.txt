SET(WO2SC3_TARGET wo2sc3)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR}) 

SET(
	WO2SC3_SOURCES
		main.cpp
		wo2sc3.cpp
)

SET(
	WO2SC3_HEADERS
		wo2sc3.h
)

INCLUDE_DIRECTORIES(.)

SC_ADD_EXECUTABLE(WO2SC3 ${WO2SC3_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${WO2SC3_TARGET} client ipgp_core)

SC_INSTALL_INIT(${WO2SC3_TARGET} ${CMAKE_SOURCE_DIR}/src/trunk/apps/templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
