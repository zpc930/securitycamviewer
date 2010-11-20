
TEMPLATE = app
TARGET = v4l2http

CONFIG += debug

INCLUDEPATH += . ../common 
DEPENDPATH  += . ../common 

MOC_DIR     = .build
OBJECTS_DIR = .build
RCC_DIR     = .build
UI_DIR      = .build


HEADERS       = ../common/JpegServer.h \
		../common/getopt/getopt.h \
		Muxer.h  \
		SimpleV4L2.h
SOURCES       = ../common/JpegServer.cpp \
		../common/getopt/getopt.cpp \
		Muxer.cpp \
		main.cpp \
		SimpleV4L2.cpp
QT           += network

# Needed for centos 5.3
LIBS += -L/opt/fontconfig-2.4.2/lib -lfontconfig
