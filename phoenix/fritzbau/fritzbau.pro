TEMPLATE = app
QT = gui core xml svg
CONFIG += qt debug warn_on console
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
FORMS = ui/mainwindow.ui
HEADERS = src/mainwindowimpl.h src/svgview.h src/pcbxml.h src/svgdomdocument.h
SOURCES = src/mainwindowimpl.cpp \
 src/main.cpp \
 src/svgview.cpp \
 src/pcbxml.cpp \
 src/svgdomdocument.cpp
RESOURCES += newfile.qrc
