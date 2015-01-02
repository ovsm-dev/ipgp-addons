SET(IPGP_GUI_LAYERS_SOURCES citylayer.cpp geometrylayer.cpp platelayer.cpp tilelayer.cpp)
SET(IPGP_GUI_LAYERS_HEADERS citylayer.h platelayer.h tilelayer.h)
SET(IPGP_GUI_LAYERS_MOC_HEADERS geometrylayer.h)
SC_SETUP_GUI_LIB_SUBDIR(IPGP_GUI_LAYERS)
