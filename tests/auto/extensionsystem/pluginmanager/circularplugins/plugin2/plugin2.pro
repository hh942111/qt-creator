TEMPLATE = lib

SOURCES += plugin2.cpp
HEADERS += plugin2.h
DEFINES += PLUGIN2_LIBRARY

OTHER_FILES = $$PWD/plugin.json

QTC_LIB_DEPENDS += extensionsystem
include(../../../../../../qtcreator.pri)
include(../../../../qttestrpath.pri)

TARGET = $$qtLibraryName(plugin2)
CONFIG -= debug_and_release_target
