cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# Version
set(boost_asio_bluetooth_VERSION_MAJOR 0)
set(boost_asio_bluetooth_VERSION_MINOR 0)
set(boost_asio_bluetooth_VERSION_PATCH 2)
set(boost_asio_bluetooth_VERSION ${boost_asio_bluetooth_VERSION_MAJOR}.${boost_asio_bluetooth_VERSION_MINOR}.${boost_asio_bluetooth_VERSION_PATCH})
message(STATUS "####### boost_asio_bluetooth_VERSION:            "   ${boost_asio_bluetooth_VERSION})

project(boost_asio_bluetooth VERSION ${boost_asio_bluetooth_VERSION} DESCRIPTION "C++ Boost::boost_asio_bluetooth 2nd party - Bluetooth for boost::asio.")
enable_language(CXX)


# Module and Packages
include(CMakePackageConfigHelpers)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Config)



IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE "Release")
   MESSAGE(STATUS ${CMAKE_BUILD_TYPE})
ENDIF()

# Find sdbus-c++
find_package(sdbus-c++ CONFIG REQUIRED)
if (sdbus-c++_FOUND)
  set(DEPS_LIBRARIES dbus-1 sdbus-c++)
  if(NOT CONAN_BUILD)    
      set(sdbus-c++_INCLUDE_DIR /opt/sdbus-c++/include)
      set(sdbus-c++_LIB_DIR /opt/sdbus-c++/lib)
      link_directories(${sdbus-c++_LIB_DIR})
  endif()
  message(STATUS "####### sdbus-c++ Package Found !")
  message(STATUS "####### sdbus-c++_VERSION:                "   ${sdbus-c++_VERSION})
  message(STATUS "####### sdbus-c++_INCLUDE_DIR:            "   ${sdbus-c++_INCLUDE_DIR})
  message(STATUS "####### sdbus-c++_DIR:                    "   ${sdbus-c++_DIR})
endif()

# Find boost
# Boost
if (NOT CONAN_BUILD)
 if (CMAKE_CROSSCOMPILING)
   message(STATUS "Cross-compiling for " ${CMAKE_LIBRARY_ARCHITECTURE})
   list(PREPEND CMAKE_PREFIX_PATH /opt/${CMAKE_LIBRARY_ARCHITECTURE}/boost/lib/)
   set(Boost_ROOT /opt/${CMAKE_LIBRARY_ARCHITECTURE}/boost)
   set(Boost_INCLUDE_DIR /opt/${CMAKE_LIBRARY_ARCHITECTURE}/boost/include)
   set(Boost_LIBRARY_DIR /opt/${CMAKE_LIBRARY_ARCHITECTURE}/boost/lib)
   set(Boost_LIBRARIES_DIR /opt/${CMAKE_LIBRARY_ARCHITECTURE}/boost/lib)
 endif()
endif()
find_package(Boost REQUIRED system thread)
if (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
    add_definitions("-DHAS_BOOST")
else(Boost_FOUND)
        message(FATAL_ERROR "Boost not found")
endif()
# Find bluetooth
find_library(bluetooth_LIBS NAMES bluetooth PATHS /usr/lib/ /usr/lib/x86_64-linux-gnu)
IF(NOT bluetooth_LIBS)
    message(FATAL_ERROR "Could not find bluetooth library. Please install libbluetooth-dev library.")
ENDIF()
SET(REQUIRED_LIBRARIES ${Boost_LIBRARIES} ${bluetooth_LIBS} pthread)
LIST(APPEND REQUIRED_LIBRARIES ${DEPS_LIBRARIES})



include_directories(${PROJECT_SOURCE_DIR}/include/)

# Source files
file(GLOB_RECURSE boost_asio_bluetooth_SRC
     "src/*.cpp"
)
# Source files
file(GLOB_RECURSE boost_asio_bluetooth_HEADERS
     "include/*.h*"
)

if (BUILD_SHARED)
  add_library(${PROJECT_NAME} SHARED ${boost_asio_bluetooth_SRC})
elseif()
  add_library(${PROJECT_NAME} STATIC ${boost_asio_bluetooth_SRC})
endif()

target_link_libraries(${PROJECT_NAME}
  Boost::system
	Boost::thread
)

MESSAGE(STATUS "REQUIRED_LIBRARIES=${REQUIRED_LIBRARIES}")

if (BUILD_DEMOS)
  add_executable(echo_server demo/echoserver.cpp src/wrapper.cpp)
  target_link_libraries(echo_server ${REQUIRED_LIBRARIES})
  target_include_directories(echo_server PRIVATE include)
  
  add_executable(echo_client demo/echoclient.cpp src/wrapper.cpp)
  target_link_libraries(echo_client ${REQUIRED_LIBRARIES})
  target_include_directories(echo_client PRIVATE include)
endif()


include(Lib)

############################################################################
# Options Summary
############################################################################
message(STATUS "############## ${PROJECT_NAME} OPTIONS SUMMARY ##############")
message(STATUS "####### BUILD_SHARED:                       " 	${BUILD_SHARED})
message(STATUS "####### BUILD_TESTS:                        " 	${BUILD_TESTS})
message(STATUS "####### BUILD_DEMOS:                        " 	${BUILD_DEMOS})
message(STATUS "####### BUILD_DOC:                          "    ${BUILD_DOC})
message(STATUS "####### CODE_COVERAGE:                      " 	${CODE_COVERAGE})
message(STATUS "####### CONAN_BUILD:                        " 	${CONAN_BUILD})
message(STATUS "####### ASAN:                               " 	${ASAN})
message(STATUS "####### TSAN:                               " 	${TSAN})