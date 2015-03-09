SET(GUI_DATAMODEL_SOURCES
    colormapviewer.cpp
    frequencyviewer.cpp
	logdialog.cpp
	plottingwidget.cpp
	qcustomstandarditem.cpp
	recordentity.cpp
	recordmanager.cpp
	recordviewer.cpp
	squarrezoomplot.cpp
	streamdelegate.cpp
)

SET(GUI_DATAMODEL_HEADERS
	misc.h
	qcustomitems.hpp
	qcustomstandarditem.h
	recordentity.h
)

SET(GUI_DATAMODEL_MOC_HEADERS
    colormapviewer.h
    frequencyviewer.h
	logdialog.h
	mainwindow.h
	plottingwidget.h
	recordmanager.h
	recordviewer.h
	squarrezoomplot.h
	streamdelegate.h
)

SET(GUI_DATAMODEL_UI logdialog.ui)

SET(GUI_DATAMODEL_RESOURCES datamodel.qrc)

SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL crosssection)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL eventenergy)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL eventlist)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL eventphaseduration)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL eventtypevariance)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL gutenbergrichter)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL hypocentersdrift)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL magnitudedensity)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL magnitudevariance)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL originplot)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL originrecordviewer)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL particlemotion)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL progressindicator)

SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL stream)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL toolbox)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL uncertainty)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL usgsfeed)
SC_ADD_GUI_SUBDIR_SOURCES(GUI_DATAMODEL wadatidiagram)

SC_SETUP_GUI_LIB_SUBDIR(GUI_DATAMODEL)
