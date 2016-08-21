#ifndef HARMONICS_CREATOR_H
#define HARMONICS_CREATOR_H

#include <QMap>
#include <QVector>

class Complex;

class HarmonicsCreator {
public:

    enum Mode {
        Z0, M2, S2, N2, K2, K1, O1, P1, Q1, MF, MM, SSA, M4, MS4
    };

    typedef QMap<Mode, Complex> Coefficients;
    typedef QMap<Mode, Complex>::const_iterator CoefficientsIterator;
    typedef QList<Mode> ModeList;
    typedef QList<double> TData;
    typedef QList<Complex> KData;

    typedef QVector<double> Amplitudes;


    HarmonicsCreator(const TData&, unsigned t0);

    const Amplitudes& levels(unsigned t0, unsigned t_delta, unsigned n);

    double frequency(Mode m) const {return m_ModeFrequency[m];}

private:

    QMap<Mode, QString> m_ModeNames;

    HarmonicsCreator(const HarmonicsCreator&);
    HarmonicsCreator& operator=(const HarmonicsCreator&);

    unsigned m_T;
    QMap<Mode, double> m_ModeFrequency;
    ModeList m_Modes;
    Amplitudes m_TData;
    KData m_KData;
    unsigned m_K0, m_KMax;
    Coefficients m_Coefficients;
    Complex m_I;

};



#endif
