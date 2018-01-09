TEMPLATE= app
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc

win32:CONFIG += x86_64 debug
win32:CONFIG -= x86 release

QT += widgets

SOURCES += main.cpp boardView.cpp board.cpp

HEADERS += boardView.h board.h
