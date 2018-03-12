#-------------------------------------------------
#
# Project created by QtCreator 2018-03-07T19:00:18
#
#-------------------------------------------------

QT       -= core gui

TARGET = libio
TEMPLATE = lib
CONFIG += staticlib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += "include"

SOURCES += \
        libio.cpp \
    src/BlockNumber.cpp \
    src/Entropy.cpp \
    src/error.cpp \
    src/factories.cpp \
    src/iofs.cpp \
    src/iofunctions.cpp \
    src/IOLibrary.cpp \
    src/MovRaw.cpp \
    src/QuickTime.cpp \
    src/SignatureTest.cpp \
    src/stdafx.cpp \
    src/utility.cpp \
    src/XorAnalyzer.cpp \
    libio.cpp

HEADERS += \
        libio.h \
    include/AbstractRaw.h \
    include/BlockNumber.h \
    include/calcRaid5Parity.h \
    include/cdw_raw.h \
    include/ChangeHeader.h \
    include/constants.h \
    include/dataarray.h \
    include/dbf.h \
    include/djvu_raw.h \
    include/DVR_raw.h \
    include/Entropy.h \
    include/error.h \
    include/ext2_raw.h \
    include/Factories.h \
    include/Finder.h \
    include/FireBird.h \
    include/func_utils.h \
    include/GoPro.h \
    include/HexTexRaw.h \
    include/IODevice.h \
    include/iofs.h \
    include/iofunctions.h \
    include/iolibrary_global.h \
    include/jpeg.h \
    include/keychainraw.h \
    include/log.h \
    include/MLV_raw.h \
    include/MovRaw.h \
    include/MovRename.h \
    include/pageaddition.h \
    include/PanasonicRaw.h \
    include/physicaldrive.h \
    include/QuickTime.h \
    include/r3d_raw.h \
    include/RawAVI.h \
    include/RawIMD.h \
    include/RawMPEG.h \
    include/RawMTS.h \
    include/RawMXF.h \
    include/RawRIFF.h \
    include/RemoveInserts.h \
    include/SignatureTest.h \
    include/StandartRaw.h \
    include/stdafx.h \
    include/targetver.h \
    include/Temp.h \
    include/Translator.h \
    include/utility.h \
    include/XorAnalyzer.h \
    src/stdafx.h \
    src/targetver.h \
    libio.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
