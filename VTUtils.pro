DESTDIR = $$shadowed($$PWD)
ROOTDIR = $$PWD

TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl
CONFIG += depend_includepath
CONFIG -= qt
CONFIG += c++11
TARGET = ArUtils

INCLUDEPATH += $$ROOTDIR/SFML/include

CONFIG(debug, debug|release) : DEFINES *= _DEBUG
CONFIG(release, debug|release) : DEFINES *= NDEBUG

DEFINES += _CRT_SECURE_NO_WARNINGS

unix {
    CONFIG += object_parallel_to_source   # prevent object name collisions
    QMAKE_CXXFLAGS_WARN_ON = ""
    QMAKE_CXXFLAGS += -Wall -Wextra
    QMAKE_CXXFLAGS += -Wno-unknown-pragmas -Wno-missing-braces -Wno-missing-field-initializers
    #QMAKE_CXXFLAGS += -pedantic   # controversial

    CONFIG(release, debug|release) {
        QMAKE_CXXFLAGS += -flto
    }
}

win32-msvc* {
    # don't create separate debug and release folders (QtCreater does this instead)
    CONFIG -= debug_and_release debug_and_release_target
    QMAKE_CFLAGS_WARN_ON -= -W3
    QMAKE_CFLAGS_WARN_ON += -W4
    DEFINES += VT_USE_COM
}

HEADERS +=\
    $$files(*.h)\
    $$files(*.hpp)\
    Algorithm.h

SOURCES += $$files(*.cpp) $$files(sqlite/*.c)

win32 {
    SOURCES -= $$files(*Lin.cpp)
}

unix {
    SOURCES -= $$files(*Win.cpp)
    SOURCES -= NtService.cpp
    HEADERS -= NtService.h
}

win32-msvc*: LIBS += -lShell32 -ldbghelp
unix : LIBS += -ldl -pthread
