TEMPLATE= app
DESTDIR = bin
OBJECTS_DIR = obj

win32:CONFIG += x86_64 debug
win32:CONFIG -= x86 release

QT += widgets

SOURCES += main.cpp view.cpp board.cpp

HEADERS += view.h board.h
