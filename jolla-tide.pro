TEMPLATE = app

QT += qml quick sql xml
CONFIG += c++11

SOURCES += main.cpp StationProvider.cpp Station.cpp Angle.cpp Amplitude.cpp Coordinates.cpp Interval.cpp  \
    TideEvent.cpp Timestamp.cpp Speed.cpp Year.cpp Dummy.cpp \
    RunningSet.cpp ActiveStations.cpp Events.cpp

HEADERS += Amplitude.h ConstituentSet.h Speed.h StationProvider.h Year.h Angle.h Coordinates.h Station.h TideEvent.h \
    Interval.h StationFactory.h Timestamp.h Dummy.h \
    RunningSet.h ActiveStations.h Events.h

DEFINES += QT_STATICPLUGIN

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

