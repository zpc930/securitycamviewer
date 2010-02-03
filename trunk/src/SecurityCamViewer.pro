
TEMPLATE = app
TARGET = securitycamviewer

INCLUDEPATH += .
DEPENDPATH  += .

MOC_DIR     = .build
OBJECTS_DIR = .build
RCC_DIR     = .build
UI_DIR      = .build


HEADERS       = MjpegClient.h \
		MainWindow.h \
		FlowLayout.h
SOURCES       = MjpegClient.cpp \
		MainWindow.cpp \
		FlowLayout.cpp \
                main.cpp
QT           += network
