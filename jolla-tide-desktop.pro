TEMPLATE = app

QT += qml quick sql xml widgets dbus
DBUS_INTERFACES += stationupdater.xml
CONFIG += c++11
TRANSLATIONS += jolla-tide_en.ts

SOURCES += desktop-app.cpp StationProvider.cpp Station.cpp Angle.cpp Amplitude.cpp Coordinates.cpp Interval.cpp  \
    TideEvent.cpp Timestamp.cpp Speed.cpp Year.cpp RunningSet.cpp ActiveStations.cpp Events.cpp Complex.cpp HarmonicsCreator.cpp \
    Database.cpp WebFactory.cpp TideForecast.cpp Skycal.cpp PointsWindow.cpp Factories.cpp

lupdate_only {
    SOURCES += AboutFactoryPage.qml EventsDelegate.qml MainPage.qml StationSearchPage.qml tide.qml \
               ActiveStationsDelegate.qml EventsPage.qml StationSearchDelegate.qml SetMarkPage.qml \
               AboutStationPage.qml
}

HEADERS += Amplitude.h ConstituentSet.h Speed.h StationProvider.h Year.h Angle.h Coordinates.h Station.h \
    TideEvent.h Interval.h StationFactory.h Timestamp.h RunningSet.h ActiveStations.h Events.h HarmonicsCreator.h Complex.h \
    Database.h WebFactory.h TideForecast.h Skycal.h PointsWindow.h Factories.h

DEFINES += QT_STATICPLUGIN

RESOURCES += qml.qrc resources.qrc

LIBS += -lxml2 -lqwt6-qt5 -lfftw3

INCLUDEPATH += /usr/include/libxml2 /usr/include/qwt6

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

