
TEMPLATE = app
TARGET = securitycamviewer

INCLUDEPATH += . ../common
DEPENDPATH  += . ../common

MOC_DIR     = .build
OBJECTS_DIR = .build
RCC_DIR     = .build
UI_DIR      = .build

FORMS	      = PlaybackDialog.ui

HEADERS       = ../common/MjpegClient.h \
		../common/getopt/getopt.h \
		CameraViewerWidget.h \
		PlaybackWidget.h \
		PlaybackDialog.h \
		MainWindow.h \
		FlowLayout.h

SOURCES       = ../common/MjpegClient.cpp \
		../common/getopt/getopt.cpp \
		CameraViewerWidget.cpp \
		PlaybackWidget.cpp \
		PlaybackDialog.cpp \
		MainWindow.cpp \
		FlowLayout.cpp \
                main.cpp

QT           += network

CONFIG += debug


opencv: {
	HEADERS += ../common/EyeCounter.h
	SOURCES += ../common/EyeCounter.cpp
	
	DEFINES += OPENCV_ENABLED
	LIBS += -L/usr/local/lib -lcv -lcxcore
}