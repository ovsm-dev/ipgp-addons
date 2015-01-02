SET(IPGP_GUI_PM_SOURCES
    particlemotionwidget.cpp
)
SET(IPGP_GUI_PM_MOC_HEADERS
    particlemotionwidget.h
    particlemotionwidget.h
)
SET(IPGP_GUI_PM_UI
    particlemotionwidget.ui
    particlemotionsettings.ui
)

SET(IPGP_GUI_PM_RESOURCES particlemotion.qrc)

SC_SETUP_GUI_LIB_SUBDIR(IPGP_GUI_PM)
