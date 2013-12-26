INCLUDEPATH += $$PWD/src
DEFINES += GSM_LIBRARY

SOURCES += \
    $$PWD/src/gsmmodem.cpp \
    $$PWD/src/gsmutil.cpp \
    $$PWD/src/gsmsmsreader.cpp \
    $$PWD/src/qcbsmessage.cpp \
    $$PWD/src/qsmsmsmessage.cpp \
    $$PWD/src/gsmsmssender.cpp

HEADERS += \
    $$PWD/src/gsm.hpp\
    $$PWD/src/gsmmodem.hpp \
    $$PWD/src/gsmsmsreader.hpp \
    $$PWD/src/qcbsmessage.hpp \
    $$PWD/src/qsmsmessage.hpp \
    $$PWD/src/qsmsmessage_p.h \
    $$PWD/src/gsmsmssender.hpp \
    $$PWD/src/qtelephonynamespace.h
