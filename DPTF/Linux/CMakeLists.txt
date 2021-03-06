project(DPTF)

cmake_minimum_required(VERSION 2.8.6)

message ("CXX Flags (initial) : " ${CMAKE_CXX_FLAGS}) 

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wno-unused-value -fPIC -Wformat -Wformat-security -O2 -D_FORTIFY_SOURCE=2")
if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0") 
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -z noexecstack -z relro -z now")

if (CHROMIUM_BUILD MATCHES YES)
	message("Building for Chromium OS...")

	if (BUILD_ARCH MATCHES 32bit)	
	        message("Building 32-bit...")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32)
	        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32)
	        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32)
	
	else()	
	        message("Building 64-bit...")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
	        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64)
	        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64)
	        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64)
	endif()

else()
	message("Building for Linux...")

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-Os")
	
	if (BUILD_ARCH MATCHES 32bit AND CMAKE_BUILD_TYPE MATCHES Release)
        	message("Building 32-bit release...")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/release)
	        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/release)
	        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/release)

	elseif (BUILD_ARCH MATCHES 64bit AND CMAKE_BUILD_TYPE MATCHES Debug)
	        message("Building 64-bit debug...")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
	        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/debug)
	        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/debug)
	        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/debug)

	elseif (BUILD_ARCH MATCHES 32bit AND CMAKE_BUILD_TYPE MATCHES Debug)
	        message("Building 32-bit debug...")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/debug)
	        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/debug)
	        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/debug)

	else ()
	        message("Building 64-bit release...")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
	        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/release)
	        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/release)
	        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/release)
	endif()	
endif()

message ("GCC version : " ${CMAKE_CXX_COMPILER_VERSION})
message ("CXX Flags (final) : " ${CMAKE_CXX_FLAGS}) 
message ("CXX Debug Flags : " ${CMAKE_CXX_FLAGS_DEBUG}) 
message ("CXX Release Flags : " ${CMAKE_CXX_FLAGS_RELEASE}) 
#message ("CXX MinSizeRel Flags : " ${CMAKE_C_FLAGS_MINSIZEREL}) 

add_definitions(-DESIF_ATTR_OS_LINUX -DESIF_ATTR_USER)

set(BASIC_TYPES_LIB "DptfBasicTypesLib")
set(ESIF_TYPES_LIB "DptfEsifTypesLib")
set(DPTF_TYPES_LIB "DptfDptfTypesLib")
set(DPTF_OBJECTS_LIB "DptfDptfObjectsLib")
set(XML_LIB "DptfXmlLib")
set(EVENTS_LIB "DptfEventsLib")
set(MESSAGE_LOGGING_LIB "DptfMessageLoggingLib")
set(PARTICIPANT_CONTROLS_LIB "DptfParticipantControlsLib")
set(PARTICIPANT_LIB "DptfParticipantLib")
set(SHARED_LIB "DptfSharedLib")
add_subdirectory(SharedLib)

set(POLICY_LIB "DptfPolicyLib")
add_subdirectory(Policies)

set(UNIFIED_PARTICIPANT "DptfParticipant")
add_subdirectory(UnifiedParticipant)

set(MANAGER "Dptf")
add_subdirectory(Manager)
