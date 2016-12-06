TEMPLATE = app

QT += sql xml dbus network widgets
DBUS_ADAPTORS += stationupdater.xml
CONFIG += c++11

SOURCES += harmonics.cpp Angle.cpp Amplitude.cpp Coordinates.cpp Interval.cpp  \
    Timestamp.cpp Speed.cpp Year.cpp RunningSet.cpp Complex.cpp HarmonicsCreator.cpp \
    Database.cpp Address.cpp PatchIterator.cpp PointsWindow.cpp \
    Station.cpp Skycal.cpp

HEADERS += Amplitude.h ConstituentSet.h Speed.h Year.h Angle.h Coordinates.h \
    Interval.h Timestamp.h RunningSet.h HarmonicsCreator.h Complex.h \
    Database.h Address.h PatchIterator.h PointsWindow.h \
    Station.h Skycal.h

RESOURCES += harmonics.qrc

LIBS += -lqwt6-qt5 -lfftw3

INCLUDEPATH += /usr/include/eigen3 /usr/include/qwt6

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

