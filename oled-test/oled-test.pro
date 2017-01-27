TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

xpm-output: DEFINES += XPM_OUTPUT

SOURCES += \
    gpio.cpp \
    spidevice.cpp \
    main.cpp \
    oledcdisplay.cpp \
    xpm.cpp

HEADERS += \
    gpio.h \
    spidevice.h \
    seps114a.h \
    oledcdisplay.h \
    xpm.h
