# - Try to find the JSONCPP library
# Once done this will define
#
# JSONCPP_FOUND - system has JSONCPP library
# JSONCPP_LIBRARIES - JSONCPP libraries directory
# JSONCPP_INCLUDE_DIR - JSONCPP include directory

# Copyright (c) 2019, DreamOverflow, <frederic.scherma@dreamoverflow.org>
#

if (UNIX)
    if(JSONCPP_LIBRARIES)
        set(JSONCPP_FIND_QUIETLY TRUE)
    endif(JSONCPP_LIBRARIES)

    find_library(JSONCPP_LIBRARY jsoncpp)
    set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})

    find_path(JSONCPP_INCLUDE_DIR jsoncpp)

    # handle the QUIETLY and REQUIRED arguments and set
    # JSONCPP_FOUND to TRUE if all listed variables are TRUE
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(jsoncpp DEFAULT_MSG JSONCPP_INCLUDE_DIR JSONCPP_LIBRARY)
endif (UNIX)
