cmake_minimum_required (VERSION 3.2)
project (SIIS_STRATEGY)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   message(STATUS "Setting build type to 'Debug' as none was specified.")
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
   # Set the possible values of build type for cmake-gui
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
     "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT MINGW AND ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # compiler options
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /D_DEBUG /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
    set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /DNDEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER /DO3D_DISABLE_GPU_MEMORY_MANAGER /DO3D_DISABLE_SPU_MEMORY_MANAGER")
    set(CMAKE_CXX_FLAGS_DEBUG          "/D_DEBUG /Zi /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
    set(CMAKE_CXX_FLAGS_DEBUGFULL      "/D_DEBUG /Zi /EHsc /MP /MDd")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/O2 /Zi /D_DEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
    set(CMAKE_C_FLAGS_RELEASE          "/O2 /DNDEBUG /EHsc /MP /MD /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER /DO3D_DISABLE_GPU_MEMORY_MANAGER /DO3D_DISABLE_SPU_MEMORY_MANAGER")
    set(CMAKE_C_FLAGS_DEBUG            "/D_DEBUG /Zi /EHsc /MP /MDd /DO3D_DISABLE_RAM_MEMORY_MANAGER /DO3D_DISABLE_FAST_MEMORY_MANAGER")
    set(CMAKE_C_FLAGS_DEBUGFULL        "/D_DEBUG /Zi /EHsc /MP /MDd")
else()
    # compiler options
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fvisibility=hidden")

    # until we have fixed any missing override keyword waited by c++17
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-inconsistent-missing-override")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -ffunction-sections -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -glldb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -glldb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_CXX_FLAGS_DEBUGFULL      "-O0 -g -glldb -D_DEBUG")
        set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -ffunction-sections -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -glldb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_C_FLAGS_DEBUG            "-O0 -g -glldb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_C_FLAGS_DEBUGFULL        "-O0 -g -glldb -D_DEBUG")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG -ffunction-sections -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_CXX_FLAGS_DEBUGFULL      "-O0 -g -ggdb -D_DEBUG")
        set(CMAKE_C_FLAGS_RELEASE          "-O2 -DNDEBUG -ffunction-sections -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER -DO3D_DISABLE_GPU_MEMORY_MANAGER -DO3D_DISABLE_SPU_MEMORY_MANAGER")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_C_FLAGS_DEBUG            "-O0 -g -ggdb -D_DEBUG -DO3D_DISABLE_RAM_MEMORY_MANAGER -DO3D_DISABLE_FAST_MEMORY_MANAGER")
        set(CMAKE_C_FLAGS_DEBUGFULL        "-O0 -g -ggdb -D_DEBUG")
    endif ()

    # linker options
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE  "-Wl,--gc-sections")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE  "-Wl,--gc-sections")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE  "-Wl,--gc-sections")
#    if(NOT MINGW)
#        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE  "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,-fuse-ld=gold")
#        set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -Wl,-fuse-ld=gold")
#        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-fuse-ld=gold")
#    endif()
endif()

#----------------------------------------------------------
# externals libraries
#----------------------------------------------------------

include(dreamoverflow)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH} $ENV{PREFIX})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_PREFIX_PATH $ENV{PREFIX} ${CMAKE_PREFIX_PATH})
	set(CMAKE_INCLUDE_PATH $ENV{PREFIX}/include ${CMAKE_INCLUDE_PATH})
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

find_package(Objective3D REQUIRED)
find_package(TA REQUIRED)
find_package(ZMQ REQUIRED)
find_package(JSONCPP REQUIRED)
# find_package(PostgreSQL REQUIRED)

include_directories(${JSONCPP_INCLUDE_DIR})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(LIBRARY_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BUILD_TYPE}/bin)

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")

#----------------------------------------------------------
# options
#----------------------------------------------------------

set(O3D_USE_SSE2 1)

include_directories(${OBJECTIVE3D_INCLUDE_DIR})
include_directories(${OBJECTIVE3D_INCLUDE_DIR_objective3dconfig})

link_directories(${OBJECTIVE3D_LIBRARY_DIR})

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -msse2")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -msse2")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-parameter -std=c++14 -fexceptions -Wextra")
	if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
	else()
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
	endif()

	# Windows subsystem
	if(MSVC)
		set(CMAKE_CREATE_WIN32_EXE "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
	endif(MSVC)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#----------------------------------------------------------
# sub-directories
#----------------------------------------------------------

set(LIBRARY_OUTPUT_PATH ../lib)
set(EXECUTABLE_OUTPUT_PATH ../bin)

add_subdirectory(src)

#----------------------------------------------------------
# resources
#----------------------------------------------------------

# could be usefull for some IDEs based on cmake to retrieve src for debugging
# @todo how to do that using IMPORT/EXPORT library options
# include_directories(${CMAKE_BINARY_DIR}/../../src)

