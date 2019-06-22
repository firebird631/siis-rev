# - Try to find the TA library
# Once done this will define
#
# TA_FOUND - system has TA library
# TA_LIBRARIES - TA libraries directory

# Copyright (c) 2019, DreamOverflow, <frederic.scherma@dreamoverflow.org>
#

if (UNIX)
	if(TA_LIBRARIES)
	    set(TA_FIND_QUIETLY TRUE)
	endif(TA_LIBRARIES)

	find_library(TA_LIBRARY ta_lib)
	set(TA_LIBRARIES ${TA_LIBRARY})
	# handle the QUIETLY and REQUIRED arguments and set
	# TA_FOUND to TRUE if all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(ta_lib DEFAULT_MSG TA_LIBRARY)
endif (UNIX)
