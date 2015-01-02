SET(IPGP_CORE_SOURCES
	objectcache.cpp
	recordcache.cpp
	recordsequencer.cpp
)

SET(IPGP_CORE_HEADERS
	flags.h
	objectcache.h
	recordcache.h
	recordsequencer.h
	types.h
)

SC_SETUP_LIB_SUBDIR(IPGP_CORE)
