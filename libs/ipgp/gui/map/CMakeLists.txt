SET(GUI_MAP_SOURCES
	canvas.cpp
	decorator.cpp
	geometry.cpp
	layer.cpp
	mapwidget.cpp
	tile.cpp
	util.cpp
)

SET(GUI_MAP_HEADERS
	config.h
	decorator.h
	geometry.h
	mapdescriptor.hpp
	tile.h
	util.h
)

SET(GUI_MAP_MOC_HEADERS
	canvas.h
	layer.h
	mapwidget.h
)

SET(GUI_MAP_UI)

SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP canvases)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP decorators)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP drawables)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP geometries)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP layers)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP widgets)

SET(GUI_MAP_RESOURCES map.qrc)

SC_SETUP_GUI_LIB_SUBDIR(GUI_MAP)
