TEMPLATE= app
DESTDIR = bin
OBJECTS_DIR = obj

win32:CONFIG += x86_64 debug
win32:CONFIG -= x86 release

QT += widgets

SOURCES += main.cpp boardView.cpp board.cpp

HEADERS += boardView.h board.h
