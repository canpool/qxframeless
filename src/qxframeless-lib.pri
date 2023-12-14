VERSION = 0.0.1

QT += widgets

equals(QX_FRAMELESS_CONFIG_STATIC_LIBRARY, 1) {
    DEFINES += QX_FRAMELESS_LIBRARY_STATIC
} else {
    DEFINES += QX_FRAMELESS_LIBRARY
}

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/framelesshelper.h \
    $$PWD/framelesshelper_p.h \
    $$PWD/qxframeless_global.h

SOURCES += \
    $$PWD/framelesshelper.cpp
