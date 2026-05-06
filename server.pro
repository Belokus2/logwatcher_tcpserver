TEMPLATE = app
TARGET = server
QT += core network
CONFIG += c++17 console

SOURCES += \
    filewatcher.cpp \
    logparser.cpp \
    logstorage.cpp \
    main.cpp \
    statsengine.cpp \
    tcpserver.cpp

HEADERS += \
    filewatcher.h \
    logparser.h \
    logstorage.h \
    statsengine.h \
    tcpserver.h \
    utils.h
