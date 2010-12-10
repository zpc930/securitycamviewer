
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
		../common/getopt/getopt.h \
		Muxer.h 
SOURCES       = ../common/MjpegClient.cpp \
		../common/JpegServer.cpp \
		../common/getopt/getopt.cpp \
		Muxer.cpp \
		main.cpp
QT           += network

opencv: {
	HEADERS += ../common/EyeCounter.h
	SOURCES += ../common/EyeCounter.cpp
	
	DEFINES += OPENCV_ENABLED
	LIBS += -L/usr/local/lib -lcv -lcxcore
}

# Needed for centos 5.3
LIBS += -L/opt/fontconfig-2.4.2/lib -lfontconfig
