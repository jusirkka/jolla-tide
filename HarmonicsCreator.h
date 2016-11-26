#ifndef HARMONICS_CREATOR_H
#define HARMONICS_CREATOR_H

#include <QHash>
#include <QMap>
#include <QVector>
#include "RunningSet.h"
#include "PatchIterator.h"

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

    typedef QVector<Speed> Speeds;

    static RunningSet* CreateConstituents(int station_id);
    static const ModeName& Modes();
    static void Config(const QString& key, const QVariant& value);
    static void Delete(db_int_t station_id);



private:


    HarmonicsCreator(const HarmonicsCreator&);
    HarmonicsCreator& operator=(const HarmonicsCreator&);
    HarmonicsCreator();


    static HarmonicsCreator* instance();

    void config(const QString& key, const QVariant& value);

    ModeSpeed modes() const {return m_KnownModes;}
    ModeName names() const {return m_KnownNames;}

    void reset(db_int_t station_id);
    void average(Coefficients& coeffs, Timestamp& epoch);
    double errorEstimate(const Coefficients& coeffs);
    Coefficients solve();
    Speeds selectModes();
    bool checkModes(Speeds& modes);
    void checkDBIntegrity();
    void select(db_int_t station_id, Coefficients& coeffs, Timestamp& epoch);
    void insert(db_int_t station_id, const Coefficients& coeffs, const Timestamp& epoch);

    void printMatrix(const ModeMatrix& m);
    void computeMatrix(ModeMatrix& m, bool diag = true) const;
    Complex computeElement(const Speed& q, const Speed& p, bool diag) const;
    Complex coeff(double omega) const;
    double factor(double x, unsigned n) const;
    Coefficients fitModes(Speeds& selected);

private:

    Complex m_I;
    ModeSpeed m_KnownModes;
    ModeName m_KnownNames;
    Coefficients m_Averages;
    Patch m_Patch;
    PatchIterator* m_Data;

    double m_AmplitudeCut;
    double m_SlowCut;
    double m_ResolutionCut;
    double m_AmplitudeDiffLowerCut;
    double m_AmplitudeDiffUpperCut;
    db_int_t m_MaxSampleSize;

};

}

#endif
