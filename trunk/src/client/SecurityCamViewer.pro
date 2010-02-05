
TEMPLATE = app
TARGET = securitycamviewer

INCLUDEPATH += . ../common
DEPENDPATH  += . ../common

MOC_DIR     = .build
OBJECTS_DIR = .build
RCC_DIR     = .build
UI_DIR      = .build


HEADERS       = ../common/MjpegClient.h \
		../common/getopt/getopt.h \
		CameraViewerWidget.h \
		PlaybackWidget.h \
		MainWindow.h \
		FlowLayout.h
SOURCES       = ../common/MjpegClient.cpp \
		../common/getopt/getopt.cpp \
		CameraViewerWidget.cpp \
		PlaybackWidget.cpp \
		MainWindow.cpp \
		FlowLayout.cpp \
                main.cpp
QT           += network
