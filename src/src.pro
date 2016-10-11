TEMPLATE = lib
TARGET = qhtmlparser
QT += core
QT -= gui

DEFINES += QHTMLPARSER_LIBRARY

DESTDIR = .

HEADERS += \
    qhtmlparser.h

SOURCES += \
    qhtmlparser.cpp

headers.files = \
    qhtmlparser.h

maemo5 {
    INCLUDEPATH += /usr/include/tidy
}

unix {
    CONFIG += link_prl
    LIBS += -L/usr/lib -ltidy
}

contains(DEFINES, QHTMLPARSER_STATIC_LIBRARY) {
    CONFIG += staticlib
} else {
    CONFIG += create_prl
    INSTALLS += target headers

    !isEmpty(INSTALL_SRC_PREFIX) {
        target.path = $$INSTALL_SRC_PREFIX/lib
        headers.path = $$INSTALL_SRC_PREFIX/include/qhtmlparser
    } else {
        target.path = /usr/lib
        headers.path = /usr/include/qhtmlparser
    }
}
