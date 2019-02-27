TEMPLATE= app
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc

CONFIG += debug

win32:CONFIG += x86_64
win32:CONFIG -= x86 release

QT += widgets network

SOURCES += main.cpp utils.cpp boardView.cpp board.cpp gameView.cpp game.cpp playerView.cpp die.cpp resourceSelectors.cpp \
    messenger.cpp chatWidget.cpp

HEADERS += utils.h viewUtils.h boardView.h board.h gameView.h game.h playerView.h die.h resourceSelectors.h \
    messenger.h chatWidget.h
