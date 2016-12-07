TEMPLATE = app

QT += qml quick sql xml widgets dbus
CONFIG += c++11
TRANSLATIONS += jolla-tide_en.ts

TOP = ../..

TSRC = $${TOP}/src
FILES = $${TOP}/files

DBUS_INTERFACES += $${FILES}/stationupdater.xml

SOURCES += $${TSRC}/Angle.cpp $${TSRC}/Amplitude.cpp $${TSRC}/Coordinates.cpp \
    $${TSRC}/Interval.cpp $${TSRC}/Timestamp.cpp $${TSRC}/Speed.cpp $${TSRC}/Year.cpp \
    $${TSRC}/RunningSet.cpp $${TSRC}/Complex.cpp $${TSRC}/HarmonicsCreator.cpp \
    $${TSRC}/Database.cpp $${TSRC}/Address.cpp $${TSRC}/PatchIterator.cpp $${TSRC}/PointsWindow.cpp \
    $${TSRC}/Station.cpp $${TSRC}/Skycal.cpp $${TSRC}/StationProvider.cpp $${TSRC}/TideEvent.cpp $${TSRC}/ActiveStations.cpp \
    $${TSRC}/Events.cpp $${TSRC}/WebFactory.cpp $${TSRC}/TideForecast.cpp $${TSRC}/Factories.cpp \
    main.cpp

HEADERS += $${TSRC}/Amplitude.h $${TSRC}/ConstituentSet.h $${TSRC}/Speed.h $${TSRC}/Year.h \
    $${TSRC}/Angle.h $${TSRC}/Coordinates.h $${TSRC}/Interval.h $${TSRC}/Timestamp.h \
    $${TSRC}/RunningSet.h $${TSRC}/HarmonicsCreator.h $${TSRC}/Complex.h \
    $${TSRC}/Database.h $${TSRC}/Address.h $${TSRC}/PatchIterator.h $${TSRC}/PointsWindow.h \
    $${TSRC}/Station.h $${TSRC}/Skycal.h $${TSRC}/StationProvider.h $${TSRC}/TideEvent.h $${TSRC}/StationFactory.h \
    $${TSRC}/ActiveStations.h $${TSRC}/Events.h $${TSRC}/WebFactory.h $${TSRC}/TideForecast.h $${TSRC}/Factories.h

#lupdate_only {
#    SOURCES += AboutFactoryPage.qml EventsDelegate.qml MainPage.qml StationSearchPage.qml tide.qml \
#               ActiveStationsDelegate.qml EventsPage.qml StationSearchDelegate.qml SetMarkPage.qml \
#               AboutStationPage.qml
#}

DEFINES += QT_STATICPLUGIN

RESOURCES += qml.qrc translations.qrc $${TOP}/icons.qrc $${TOP}/harmonics.qrc

LIBS += -lxml2 -lqwt6-qt5 -lfftw3

INCLUDEPATH += /usr/include/libxml2 /usr/include/qwt6 /usr/include/eigen3 $${TSRC}

