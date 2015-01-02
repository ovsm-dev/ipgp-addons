SET(GUI_OPENGL_SOURCES
	canvas.cpp
	renderer.cpp	
	camera.cpp
	gl.cpp
	graphicrenderer.cpp
	mmath.cpp
	topographyfile.cpp
	topographyrenderer.cpp
	topographyrenderersettings.cpp
	triangle.cpp
	vertex.cpp
)

SET(GUI_OPENGL_HEADERS
    camera.h
   	gl.h
	mmath.h
	topographyfile.h
	topographyrenderersettings.h
	triangle.h
	vertex.h
)

SET(GUI_OPENGL_MOC_HEADERS
	canvas.h
	graphicrenderer.h
	renderer.h
	topographyrenderer.h
)

SET(GUI_OPENGL_UI)

SC_ADD_GUI_SUBDIR_SOURCES(GUI_OPENGL drawables)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_OPENGL widgets)

SET(GUI_OPENGL_RESOURCES opengl.qrc)

SC_SETUP_GUI_LIB_SUBDIR(GUI_OPENGL)
