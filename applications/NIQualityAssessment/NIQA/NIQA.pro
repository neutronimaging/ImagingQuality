#-------------------------------------------------
#
# Project created by QtCreator 2017-08-12T08:41:01
#
#-------------------------------------------------

QT       += core gui charts svg # printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NIQA
TEMPLATE = app

CONFIG += c++11

VERSION = 0.1.0
DEFINES += VERSION=\\\"$$VERSION\\\"

CONFIG(release, debug|release): DESTDIR = $$PWD/../../../../Applications
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../../../Applications/debug

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_PRINTER

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix {
    target.path = /usr/lib
    INSTALLS += target

    unix:macx {
        QMAKE_CXXFLAGS += -fPIC -O2
        INCLUDEPATH += /opt/local/include

        INCLUDEPATH += /opt/local/include/libxml2
        QMAKE_LIBDIR += /opt/local/lib
    }
    else {
        QMAKE_CXXFLAGS += -fPIC -fopenmp -O2
        QMAKE_LFLAGS += -lgomp
        LIBS += -lgomp
        INCLUDEPATH += /usr/include/libxml2
    }

    LIBS += -ltiff -lxml2 -larmadillo -llapack

    INCLUDEPATH += $$PWD/../../../../imagingsuite/external/src/linalg
}

win32 {
    CONFIG += console
    contains(QMAKE_HOST.arch, x86_64):{
        QMAKE_LFLAGS += /MACHINE:X64
    }

    QMAKE_LIBDIR += $$PWD/../../../../ExternalDependencies/windows/lib
    INCLUDEPATH  += $$PWD/../../../../ExternalDependencies/windows/include/cfitsio
    INCLUDEPATH  += $$PWD/../../../../ExternalDependencies/windows/include/libxml2

    INCLUDEPATH  += $$PWD/../../../../imagingsuite/external/src/linalg $$PWD/../../../../imagingsuite/external/include
    QMAKE_LIBDIR += $$_PRO_FILE_PWD_/../../../../imagingsuite/external/lib64

    LIBS += -llibxml2 -llibtiff -lcfitsio
    LIBS += -llibopenblas
    QMAKE_CXXFLAGS += /openmp /O2
}

SOURCES += main.cpp\
        niqamainwindow.cpp \
    edgefileitemdialog.cpp \
    niqaconfig.cpp \
    edgefittingdialog.cpp \
    reportmaker.cpp

HEADERS  += niqamainwindow.h \
    edgefileitemdialog.h \
    niqaconfig.h \
    edgefittingdialog.h \
    reportmaker.h

FORMS    += niqamainwindow.ui \
    edgefileitemdialog.ui \
    edgefittingdialog.ui

ICON = NIQAIcon.icns
RC_ICONS = NIQAIcon.ico

CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../lib/
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../lib/debug

LIBS += -lkipl -lQtAddons -lImagingAlgorithms -lImagingQAAlgorithms -lReaderConfig -lReaderGUI -lQtImaging

INCLUDEPATH += $$PWD/../../../../imagingsuite/core/kipl/kipl/include
DEPENDPATH += $$PWD/../../../../imagingsuite/core/kipl/kipl/include

INCLUDEPATH += $$PWD/../../../../imagingsuite/GUI/qt/QtModuleConfigure
DEPENDPATH += $$PWD/../../../../imagingsuite/GUI/qt/QtModuleConfigure

INCLUDEPATH += $$PWD/../../../../imagingsuite/GUI/qt/QtImaging
DEPENDPATH += $$PWD/../../../../imagingsuite/GUI/qt/QtImaging

INCLUDEPATH += $$PWD/../../../../imagingsuite/GUI/qt/QtAddons
DEPENDPATH += $$PWD/../../../../imagingsuite/GUI/qt/QtAddons

INCLUDEPATH += $$PWD/../../../../imagingsuite/frameworks/tomography/Framework/ReconFramework/include
DEPENDPATH += $$PWD/../../../../imagingsuite/frameworks/tomography/Framework/ReconFramework/src

INCLUDEPATH += $$PWD/../../../../imagingsuite/core/modules/ModuleConfig/include
DEPENDPATH += $$PWD/../../../../imagingsuite/core/modules/ModuleConfig/include

INCLUDEPATH += $$PWD/../../../../imagingsuite/core/modules/ReaderConfig/
DEPENDPATH += $$PWD/../../../../imagingsuite/core/modules/ReaderConfig/

INCLUDEPATH += $$PWD/../../../../imagingsuite/core/modules/ReaderGUI/
DEPENDPATH += $$PWD/../../../../imagingsuite/core/modules/ReaderGUI/

INCLUDEPATH += $$PWD/../../../../imagingsuite/core/algorithms/ImagingAlgorithms/include
DEPENDPATH += $$PWD/../../../../imagingsuite/core/algorithms/ImagingAlgorithms/src

INCLUDEPATH += $$PWD/../../../../imagingsuite/core/algorithms/ImagingQAAlgorithms
DEPENDPATH += $$PWD/../../../../imagingsuite/core/algorithms/ImagingQAAlgorithms
