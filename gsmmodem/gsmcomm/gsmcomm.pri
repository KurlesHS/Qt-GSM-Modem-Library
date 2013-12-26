INCLUDEPATH += $$PWD/src
DEFINES += GSM_LIBRARY
HEADERS += \
    $$PWD/src/gsmcomm_global.hpp \
    $$PWD/src/qatchat.h \
    $$PWD/src/qatchat_p.h \
    $$PWD/src/qatchatscript.h \
    $$PWD/src/qatresult.h \
    $$PWD/src/qatresultparser.h \
    $$PWD/src/qatutils.h \
    $$PWD/src/qgsmcodec.h \
    $$PWD/src/qprefixmatcher_p.h \
    $$PWD/src/qretryatchat.h

SOURCES += \
    $$PWD/src/qatchat.cpp \
    $$PWD/src/qatchatscript.cpp \
    $$PWD/src/qatresult.cpp \
    $$PWD/src/qatresultparser.cpp \
    $$PWD/src/qatutils.cpp \
    $$PWD/src/qgsmcodec.cpp \
    $$PWD/src/qprefixmatcher.cpp \
    $$PWD/src/qretryatchat.cpp
