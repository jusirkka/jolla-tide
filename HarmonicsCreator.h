#ifndef HARMONICS_CREATOR_H
#define HARMONICS_CREATOR_H

#include <QHash>
#include <QMap>
#include <QVector>
#include "RunningSet.h"
#include "ConstituentManager.h"

namespace Tide {

class Complex;

class HarmonicsCreator {
public:

    typedef QMap<QString, Speed> ModeSpeed;
    typedef QMapIterator<QString, Speed> ModeSpeedIterator;

    typedef QMap<Speed, QString> ModeName;
    typedef QMapIterator<Speed, QString> ModeNameIterator;

    typedef QMap<Speed, Complex> Coefficients;
    typedef QMapIterator<Speed, Complex> CoefficientsIterator;

    typedef QMap<Speed, Complex> ModeVector;
    typedef QMapIterator<Speed, Complex> ModeVectorIterator;

    typedef QMap<Speed, ModeVector> ModeMatrix;
    typedef QMapIterator<Speed, ModeVector> ModeMatrixIterator;

    typedef QVector<double> LevelData;

    static RunningSet* CreateConstituents(int station_id, double cutOff);
    static const ModeName& Modes();



private:


    HarmonicsCreator(const HarmonicsCreator&);
    HarmonicsCreator& operator=(const HarmonicsCreator&);
    HarmonicsCreator();


    static HarmonicsCreator* instance();

    ModeSpeed modes() const {return m_KnownModes;}
    ModeName names() const {return m_KnownNames;}

    void reset(db_int_t station_id);
    RunningSet* average(double b0);
    double errorEstimate(const Coefficients& coeffs);
    Coefficients solve(double cutCeil, double cutFloor, double b0);
    QVector<QVector<Speed>> selectModes(double cutCeil, double cutFloor, double b0);
    bool checkModes(QVector<Speed>& modes, double b0);
    void checkDBIntegrity();

    void printMatrix(const ModeMatrix& m);
    void computeMatrix(ModeMatrix& m, bool diag = true) const;
    Complex computeElement(const Speed& q, const Speed& p, bool diag) const;
    Complex coeff(double omega, const Patch& patch) const;
    double factor(double x, unsigned n) const;

private:

    Complex m_I;
    ModeSpeed m_KnownModes;
    ModeName m_KnownNames;
    Coefficients m_Averages;
    PatchData m_PatchData;
    PatchIterator* m_Data;
    double m_N;


};

}

#endif
