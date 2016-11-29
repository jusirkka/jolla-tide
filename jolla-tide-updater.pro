TEMPLATE = app

TARGET = jolla-tide-updater

CONFIG += sailfishapp

QT += sql xml dbus network
DBUS_ADAPTORS += stationupdater.xml
CONFIG += c++11

SOURCES += updater.cpp Station.cpp Angle.cpp Amplitude.cpp Coordinates.cpp Interval.cpp  \
    TideEvent.cpp Timestamp.cpp Speed.cpp Year.cpp RunningSet.cpp Complex.cpp HarmonicsCreator.cpp \
    Database.cpp WebFactory.cpp TideForecast.cpp Skycal.cpp Updater.cpp Address.cpp PatchIterator.cpp

HEADERS += Amplitude.h ConstituentSet.h Speed.h Year.h Angle.h Coordinates.h Station.h \
    TideEvent.h Interval.h StationFactory.h Timestamp.h RunningSet.h HarmonicsCreator.h Complex.h \
    Database.h WebFactory.h TideForecast.h Skycal.h Updater.h Address.h PatchIterator.h

DEFINES += QT_STATICPLUGIN

RESOURCES += harmonics.qrc

LIBS += -lxml2

INCLUDEPATH += /usr/include/libxml2 /usr/include/eigen3

OTHER_FILES += rpm/jolla-tide-updater.yaml jolla-tide-updater.desktop

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

DISTFILES += \
    jolla-tide-updater.yaml \
    jolla-tide-updater.desktop

