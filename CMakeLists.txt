project( EXRCubeViewer )
cmake_minimum_required( VERSION 2.8.12 )
cmake_policy(SET CMP0043 OLD)

set ( CMAKE_BUILD_TYPE Release )
set ( CMAKE_AUTOMOC ON )
set ( CMAKE_INCLUDE_CURRENT_DIR ON )

add_definitions ( -Wall )

IF(UNIX)
  FIND_LIBRARY(OpenGL_LIBRARY GL )
ELSEIF(APPLE)
  FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
ELSEIF(WIN32)
  set(OpenGL_LIBRARY "-lopengl32" )
ENDIF ()

MARK_AS_ADVANCED (OpenGL_LIBRARY)

    # for find_package ( OpenEXR REQUIRED )
    find_package(ECM REQUIRED NO_MODULE)

    set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        ${ECM_MODULE_PATH}
        )

find_package ( OpenEXR REQUIRED )

find_package ( Qt5Core REQUIRED )
find_package ( Qt5OpenGL REQUIRED )
find_package ( Qt5Widgets REQUIRED )
find_package ( Qt5Gui REQUIRED )
find_package ( Qt5Xml REQUIRED )
find_package ( Qt5Script REQUIRED )
find_package ( QGLViewer REQUIRED )

#include ( ${QT_USE_FILE} )
include_directories (
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Script_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5OpenGL_INCLUDE_DIRS}
        ${QGLViewer_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

set ( EXRCubeViewer_HDRS
	EXRCubeViewer.h
	)

set ( EXRCubeViewer_SRCS
	EXRCubeViewer.cpp
	main.cpp
	)

# set ( EXRCubeViewer_RSCS
#         EXRCubeViewer.qrc
#         )
# QT5_ADD_RESOURCES(RSCS ${EXRCubeViewer_RSCS})

# SET ( EXRCubeViewer_FORMS
#         mainwindow.ui
#         )
# 
# QT5_WRAP_UI(UIS ${EXRCubeViewer_FORMS})

add_executable ( EXRCubeViewer ${EXRCubeViewer_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( EXRCubeViewer ${OpenGL_LIBRARY} ${QGLVIEWER_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${OpenEXR_LIBRARIES} )

qt5_use_modules(EXRCubeViewer OpenGL Widgets Gui Xml Script)
