# - Try to find the ZeroMQ library
# Once done this will define
#
# ZMQ_FOUND - system has ZeroMQ library
# ZMQ_LIBRARIES - ZeroMQ libraries directory

# Copyright (c) 2019, DreamOverflow, <frederic.scherma@dreamoverflow.org>
#

if (UNIX)
    if(ZMQ_LIBRARIES)
        set(ZMQ_FIND_QUIETLY TRUE)
    endif(ZMQ_LIBRARIES)

    find_library(ZMQ_LIBRARY zmq)
    set(ZMQ_LIBRARIES ${ZMQ_LIBRARY})
	# handle the QUIETLY and REQUIRED arguments and set
    # ZMQ_FOUND to TRUE if all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(zmq DEFAULT_MSG ZMQ_LIBRARY)
endif (UNIX)
