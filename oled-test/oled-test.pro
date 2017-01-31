TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

xpm-output: DEFINES += XPM_OUTPUT

PROJECT_TOP_SRCDIR = $$PWD/..
PROJECT_TOP_BUILDDIR = $$OUT_PWD/..

INCLUDEPATH += \
        $$PROJECT_TOP_SRCDIR/libshv/include \

DESTDIR = $$PROJECT_TOP_BUILDDIR/bin

LIBDIR = $$DESTDIR
unix: LIBDIR = $$PROJECT_TOP_BUILDDIR/lib

LIBS += \
        -L$$LIBDIR \

LIBS += \
        -lshv \

unix {
        LIBS += \
                -Wl,-rpath,\'\$\$ORIGIN/../lib\'
}

SOURCES += \
    main.cpp \

HEADERS += \

