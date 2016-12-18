TEMPLATE = app

QT += sql xml network widgets
CONFIG += c++11

TOP = ..

TSRC = $${TOP}/src
FILES = $${TOP}/files

SOURCES += $${TSRC}/Angle.cpp $${TSRC}/Amplitude.cpp $${TSRC}/Coordinates.cpp \
    $${TSRC}/Interval.cpp $${TSRC}/Timestamp.cpp $${TSRC}/Speed.cpp $${TSRC}/Year.cpp \
    $${TSRC}/RunningSet.cpp $${TSRC}/Complex.cpp $${TSRC}/HarmonicsCreator.cpp \
    $${TSRC}/Database.cpp $${TSRC}/Address.cpp $${TSRC}/PatchIterator.cpp $${TSRC}/PointsWindow.cpp \
    $${TSRC}/Station.cpp $${TSRC}/Skycal.cpp main.cpp

HEADERS += $${TSRC}/Amplitude.h $${TSRC}/ConstituentSet.h $${TSRC}/Speed.h $${TSRC}/Year.h \
    $${TSRC}/Angle.h $${TSRC}/Coordinates.h $${TSRC}/Interval.h $${TSRC}/Timestamp.h \
    $${TSRC}/RunningSet.h $${TSRC}/HarmonicsCreator.h $${TSRC}/Complex.h \
    $${TSRC}/Database.h $${TSRC}/Address.h $${TSRC}/PatchIterator.h $${TSRC}/PointsWindow.h \
    $${TSRC}/Station.h $${TSRC}/Skycal.h

RESOURCES += $${TOP}/harmonics.qrc

LIBS += -lqwt6-qt5 -lfftw3

INCLUDEPATH += /usr/include/eigen3 /usr/include/qwt6 $${TSRC}



