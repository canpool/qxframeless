TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    src \
    demos

demos.depends = src
