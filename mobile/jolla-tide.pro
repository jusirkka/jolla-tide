TEMPLATE = app

TARGET = jolla-tide
CONFIG += sailfishapp c++11
QT += qml quick sql xml dbus
DBUS_INTERFACES += stationupdater.xml


CONFIG += sailfishapp_i18n sailfishapp_i18n_idbased sailfishapp_i18n_unfinished
TRANSLATIONS += silica/tide_en.ts
i18n.path = /usr/share/jolla-tide/i18n
i18n.files = i18n/translations_en.qm
INSTALLS += i18n

SOURCES += mobile-app.cpp StationProvider.cpp Station.cpp Angle.cpp Amplitude.cpp Coordinates.cpp Interval.cpp  \
    TideEvent.cpp Timestamp.cpp Speed.cpp Year.cpp RunningSet.cpp ActiveStations.cpp Events.cpp Complex.cpp HarmonicsCreator.cpp \
    Database.cpp WebFactory.cpp TideForecast.cpp Skycal.cpp Factories.cpp Address.cpp PatchIterator.cpp

HEADERS += Amplitude.h ConstituentSet.h Speed.h StationProvider.h Year.h Angle.h Coordinates.h Station.h \
    TideEvent.h Interval.h StationFactory.h Timestamp.h RunningSet.h ActiveStations.h Events.h HarmonicsCreator.h Complex.h \
    Database.h WebFactory.h TideForecast.h Skycal.h Factories.h Address.h PatchIterator.h

DEFINES += QT_STATICPLUGIN NO_POINTSWINDOW

RESOURCES += harmonics.qrc icons.qrc silica-qml.qrc

LIBS += -lxml2

INCLUDEPATH += /usr/include/libxml2 /usr/include/eigen3

OTHER_FILES += rpm/jolla-tide.yaml \
    jolla-tide.desktop

DISTFILES += \
    jolla-tide.yaml \
    jolla-tide.desktop

