TEMPLATE = app
TARGET = deployqt
DEPENDPATH += .
INCLUDEPATH += .


# Input
SOURCES += main.cpp ../shared/shared.cpp
CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/debug-shared
MOC_DIR = .moc/debug-shared
CONFIG -= app_bundle