#ifndef HARMONICS_CREATOR_H
#define HARMONICS_CREATOR_H

#include <QHash>
#include <QVector>
#include "RunningSet.h"

namespace Tide {

class Complex;

class HarmonicsCreator {
public:

    typedef QHash<QString, Complex> Coefficients;
    typedef QHashIterator<QString, Complex> CoefficientsIterator;
    typedef QHash<QString, double> ModeSpeed;
    typedef QHashIterator<QString, double> ModeSpeedIterator;
    typedef QVector<double> LevelData;

    static void UpdateDB(int station_id);
    static RunningSet* CreateConstituents(int station_id);
    static Timestamp LastDataPoint(int station_id);





private:

    HarmonicsCreator(const HarmonicsCreator&);
    HarmonicsCreator& operator=(const HarmonicsCreator&);
    HarmonicsCreator();


    static HarmonicsCreator* instance();
    Coefficients patch(int step, const LevelData& points);

    void reset();
    void append(int delta, int patchsize, int step, const Coefficients& patch);
    Coefficients average() const;
    ModeSpeed modes() {return m_W;}
    void checkDBIntegrity();

    Complex m_I;
    quint64 m_N;
    quint64 m_D;
    Coefficients m_ModeSums;
    ModeSpeed m_W;


};

}

#endif
