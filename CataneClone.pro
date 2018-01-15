TEMPLATE= app
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc

win32:CONFIG += x86_64 debug
win32:CONFIG -= x86 release

QT += widgets

SOURCES += main.cpp boardView.cpp board.cpp gameView.cpp game.cpp playerView.cpp die.cpp

HEADERS += utils.h viewUtils.h boardView.h board.h gameView.h game.h playerView.h die.h
