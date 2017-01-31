message("including $$PWD")

QT += core
QT -= gui

CONFIG += C++11
CONFIG += hide_symbols

TEMPLATE = lib
TARGET = shv

isEmpty(QF_PROJECT_TOP_BUILDDIR) {
    QF_PROJECT_TOP_BUILDDIR = $$OUT_PWD/..
}
else {
    message ( QF_PROJECT_TOP_BUILDDIR is not empty and set to $$QF_PROJECT_TOP_BUILDDIR )
    message ( This is obviously done in file $$QF_PROJECT_TOP_SRCDIR/.qmake.conf )
}
message ( QF_PROJECT_TOP_BUILDDIR == '$$QF_PROJECT_TOP_BUILDDIR' )

unix:DESTDIR = $$QF_PROJECT_TOP_BUILDDIR/lib
win32:DESTDIR = $$QF_PROJECT_TOP_BUILDDIR/bin

message ( DESTDIR: $$DESTDIR )

PROJECT_TOP_SRCDIR = $$PWD/..

DEFINES += SHV_BUILD_DLL

LIBS += \
    -L$$DESTDIR \

TRANSLATIONS += \
#	libeyastsc.pl_PL.ts \

include($$PWD/src/src.pri)

