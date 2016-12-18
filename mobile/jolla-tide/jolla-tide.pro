TEMPLATE = app

TARGET = jolla-tide
CONFIG += c++11 sailfishapp
QT += qml quick sql xml dbus


TOP = $${_PRO_FILE_PWD_}/../..

TSRC = $${TOP}/src
FILES = $${TOP}/files

DBUS_INTERFACES += $${FILES}/stationupdater.xml


TRANSLATIONS += jolla-tide-en.ts
for(t, TRANSLATIONS) {
    TRANSLATIONS_IN  += $${_PRO_FILE_PWD_}/$$t
}


qm.files = $$replace(TRANSLATIONS, \.ts, .qm)
qm.path = /usr/share/$${TARGET}/translations
qm.commands += [ \"$${OUT_PWD}\" != \"$${_PRO_FILE_PWD_}\" ] && cp $${TRANSLATIONS_IN} $${OUT_PWD}
qm.commands += ; lupdate $${TSRC} $${_PRO_FILE_PWD_}/qml -ts $${TRANSLATIONS}
qm.commands += ; lrelease -idbased $${TRANSLATIONS} -qm $${OUT_PWD}/$$replace(TRANSLATIONS, \.ts, .qm)

INSTALLS += qm


ai.files = $${TARGET}.png
ai.path = /usr/share/icons/hicolor/86x86/apps
ai.commands += [ \"$${OUT_PWD}\" != \"$${_PRO_FILE_PWD_}\" ] && cp $${_PRO_FILE_PWD_}/$${TARGET}.png $${OUT_PWD}/$${TARGET}.png

INSTALLS += ai

SOURCES += $${TSRC}/Angle.cpp $${TSRC}/Amplitude.cpp $${TSRC}/Coordinates.cpp \
    $${TSRC}/Interval.cpp $${TSRC}/Timestamp.cpp $${TSRC}/Speed.cpp $${TSRC}/Year.cpp \
    $${TSRC}/RunningSet.cpp $${TSRC}/Complex.cpp $${TSRC}/HarmonicsCreator.cpp \
    $${TSRC}/Database.cpp $${TSRC}/Address.cpp $${TSRC}/PatchIterator.cpp \
    $${TSRC}/Station.cpp $${TSRC}/Skycal.cpp $${TSRC}/StationProvider.cpp $${TSRC}/TideEvent.cpp $${TSRC}/ActiveStations.cpp \
    $${TSRC}/Events.cpp $${TSRC}/WebFactory.cpp $${TSRC}/TideForecast.cpp $${TSRC}/Factories.cpp \
    CoverModel.cpp main.cpp


HEADERS += $${TSRC}/Amplitude.h $${TSRC}/ConstituentSet.h $${TSRC}/Speed.h $${TSRC}/Year.h \
    $${TSRC}/Angle.h $${TSRC}/Coordinates.h $${TSRC}/Interval.h $${TSRC}/Timestamp.h \
    $${TSRC}/RunningSet.h $${TSRC}/HarmonicsCreator.h $${TSRC}/Complex.h \
    $${TSRC}/Database.h $${TSRC}/Address.h $${TSRC}/PatchIterator.h \
    $${TSRC}/Station.h $${TSRC}/Skycal.h $${TSRC}/StationProvider.h $${TSRC}/TideEvent.h $${TSRC}/StationFactory.h \
    $${TSRC}/ActiveStations.h $${TSRC}/Events.h $${TSRC}/WebFactory.h $${TSRC}/TideForecast.h $${TSRC}/Factories.h \
    CoverModel.h


DEFINES += QT_STATICPLUGIN NO_POINTSWINDOW

RESOURCES += $${TOP}/harmonics.qrc $${TOP}/icons.qrc

LIBS += -lxml2

INCLUDEPATH += /usr/include/libxml2 $${TSRC}

OTHER_FILES += rpm/jolla-tide.yaml jolla-tide.desktop \
    qml/AboutPage.qml qml/MainPage.qml qml/AboutStationPage.qml qml/SetMarkPage.qml \
    qml/ActiveStationsDelegate.qml qml/StationSearchDelegate.qml qml/EventsDelegate.qml \
    qml/StationSearchPage.qml qml/EventsPage.qml  qml/FactoryDelegate.qml qml/CoverPage.qml \
    qml/CoverStationsDelegate.qml \
    qml/tide.qml

