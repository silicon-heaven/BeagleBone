TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    gpio.cpp \
    spidevice.cpp \
    main.cpp

HEADERS += \
    gpio.h \
    spidevice.h
