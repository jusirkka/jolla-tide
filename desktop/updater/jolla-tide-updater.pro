TEMPLATE = app

TARGET = jolla-tide-updater

CONFIG += c++11

QT += sql xml dbus network

TOP = ../..

TSRC = $${TOP}/src
FILES = $${TOP}/files

DBUS_ADAPTORS += $${FILES}/stationupdater.xml

SOURCES += $${TSRC}/Station.cpp $${TSRC}/Angle.cpp $${TSRC}/Amplitude.cpp $${TSRC}/Coordinates.cpp \
    $${TSRC}/Interval.cpp $${TSRC}/TideEvent.cpp $${TSRC}/Timestamp.cpp $${TSRC}/Speed.cpp $${TSRC}/Year.cpp \
    $${TSRC}/RunningSet.cpp $${TSRC}/Complex.cpp $${TSRC}/HarmonicsCreator.cpp \
    $${TSRC}/Database.cpp $${TSRC}/WebFactory.cpp  $${TSRC}/TideForecast.cpp  $${TSRC}/Skycal.cpp \
    $${TSRC}/Updater.cpp $${TSRC}/Address.cpp $${TSRC}/PatchIterator.cpp \
    main.cpp

HEADERS += $${TSRC}/Amplitude.h $${TSRC}/ConstituentSet.h $${TSRC}/Speed.h $${TSRC}/Year.h $${TSRC}/Angle.h \
    $${TSRC}/Coordinates.h $${TSRC}/Station.h $${TSRC}/TideEvent.h $${TSRC}/Interval.h $${TSRC}/StationFactory.h \
    $${TSRC}/Timestamp.h $${TSRC}/RunningSet.h $${TSRC}/HarmonicsCreator.h $${TSRC}/Complex.h \
    $${TSRC}/Database.h $${TSRC}/WebFactory.h $${TSRC}/TideForecast.h $${TSRC}/Skycal.h $${TSRC}/Updater.h \
    $${TSRC}/Address.h $${TSRC}/PatchIterator.h

DEFINES += QT_STATICPLUGIN

RESOURCES += $${TOP}/harmonics.qrc

LIBS += -lxml2

INCLUDEPATH += /usr/include/libxml2 /usr/include/eigen3 $${TSRC}

