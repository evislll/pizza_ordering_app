QT       += core gui widgets
RESOURCES += resources.qrc

CONFIG   += c++17
TARGET    = pizza_ordering
TEMPLATE  = app

SOURCES += \
    animatedstack.cpp \
    main.cpp \
    mainwindow.cpp \
    steprail.cpp \
    pages.cpp

HEADERS += \
    animatedstack.h \
    mainwindow.h \
    steprail.h \
    pages.h
