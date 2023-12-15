OUT_ROOT = $${OUT_PWD}/..
TARGET = $$qtLibraryTarget(qxframeless)
TEMPLATE = lib
DESTDIR = $${OUT_ROOT}/bin

win32 {
    QX_FRAMELESS_CONFIG_NATIVE = 1
}
include($$PWD/qxframeless-lib.pri)
