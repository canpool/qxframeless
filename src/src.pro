OUT_ROOT = $${OUT_PWD}/..
TARGET = $$qtLibraryTarget(qxframeless)
TEMPLATE = lib
DESTDIR = $${OUT_ROOT}/bin

include($$PWD/qxframeless-lib.pri)
