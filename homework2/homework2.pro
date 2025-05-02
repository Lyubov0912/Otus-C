TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

LIBS += -L$$PWD/libcurl/lib -lcurl
LIBS += -L$$PWD/libcjson/lib -lcjson


INCLUDEPATH += $$PWD/libcurl/include $$PWD/libcjson/include


DEFINES += CURL_STATICLIB




