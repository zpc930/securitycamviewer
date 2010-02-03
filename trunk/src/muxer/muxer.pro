
TEMPLATE = app
TARGET = securitycam-muxer

CONFIG += debug

INCLUDEPATH += . ../common
DEPENDPATH  += . ../common

MOC_DIR     = .build
OBJECTS_DIR = .build
RCC_DIR     = .build
UI_DIR      = .build


HEADERS       = ../common/MjpegClient.h \
		../common/JpegServer.h \
		Muxer.h 
SOURCES       = ../common/MjpegClient.cpp \
		../common/JpegServer.cpp \
		Muxer.cpp \
		main.cpp
QT           += network
