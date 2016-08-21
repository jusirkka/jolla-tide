#include "Complex.h"
#include "HarmonicsCreator.h"
#include <cmath>
#include <malloc.h>
#include <cstdio>
#include <iostream>
#include <QTextStream>
#include <fftw3.h>
#include <QString>

HarmonicsCreator::HarmonicsCreator(const TData& levels, unsigned t0): m_I(Complex(0, 1)) {
    double c = 1.0 / 360 / 3600;

    m_ModeFrequency[Z0] = 0;
    m_ModeFrequency[M2] = c * 28.9841042;
    m_ModeFrequency[S2] = c * 30;
    m_ModeFrequency[N2] = c * 28.4397295;
    m_ModeFrequency[K2] = c * 30.0821373;
    m_ModeFrequency[K1] = c * 15.0410686;
    m_ModeFrequency[O1] = c * 13.9430356;
    m_ModeFrequency[P1] = c * 14.9589314;
    m_ModeFrequency[Q1] = c * 13.3986609;
    m_ModeFrequency[MF] = c * 1.0980331;
    m_ModeFrequency[MM] = c * 0.5443747;
    m_ModeFrequency[SSA] = c * 0.0821373;
    m_ModeFrequency[M4] = c * 57.9682084;
    m_ModeFrequency[MS4] = c * 58.9841042;

    m_ModeNames[Z0]= "Zero";
    m_ModeNames[M2]= "M2";
    m_ModeNames[S2]= "S2";
    m_ModeNames[N2]= "N2";
    m_ModeNames[K2]= "K2";
    m_ModeNames[K1]= "K1";
    m_ModeNames[O1]= "O1";
    m_ModeNames[P1]= "P1";
    m_ModeNames[Q1]= "Q1";
    m_ModeNames[MF]= "MF";
    m_ModeNames[MM]= "MM";
    m_ModeNames[SSA]= "SSA";
    m_ModeNames[M4]= "M4";
    m_ModeNames[MS4]= "MS4";

    int count = 2 * (levels.length() / 2);

    if (count < 2) {
        return;
    }

    if ((count * t0) % 43200 == 0) {
        // S2 resonance prevention
        count -= 2;
    }


    m_T = t0;
    m_K0 = 2;
    m_KMax = count * 70 * t0 / 360 / 3600;

    QList<unsigned> kvalues;
    foreach (Mode m, m_ModeFrequency.keys()) {
        double kvalue = m_ModeFrequency[m] * count * t0;
        if (m != Z0 && kvalue < m_K0 || kvalues.contains(unsigned(kvalue))) {
            std::cerr << "skipping " << m_ModeNames[m].toAscii().constData() << ", k_c = " << kvalue << std::endl;
            continue;
        }
        std::cerr << "adding " << m << " ("<< m_ModeNames[m].toAscii().constData() << ") , k_c = " << kvalue << std::endl;
        kvalues.append(unsigned(kvalue));
        m_Modes.append(m);
    }

    double *dp = (double *)malloc(sizeof(double) * (count + 2));
    fftw_complex *cp = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * count);

    fftw_plan plan = fftw_plan_dft_r2c_1d(2^16, dp, cp, FFTW_MEASURE);

    for(int i = 0; i < count; i++) {
        dp[i] = levels[i];
    }

    fftw_execute(plan);
    for (int k = 0; k < count / 2; k++) {
        m_KData << Complex(cp[k][0], cp[k][1]);
    }

    fftw_destroy_plan(plan);

    foreach(Mode mode, m_Modes) {
        if (mode == Z0) {
            double sum = 0;
            for (int n = 0; n < levels.count(); n++) {
                sum += levels[n];
            }
            m_Coefficients[mode] = sum / levels.count();
        } else {
            Complex sum = 0;
            for (int n = 0; n < levels.count(); n++) {
                sum += levels[n] * exp(-2 * M_PI * m_I * m_ModeFrequency[mode] * m_T * n);
            }
            m_Coefficients[mode] = 2 * sum / levels.count();
        }
        std::cerr << m_ModeNames[mode].toAscii().constData() << " = "
                  << m_Coefficients[mode].mod() << ", " << m_Coefficients[mode].arg()
                  << std::endl;
    }

}


const HarmonicsCreator::Amplitudes& HarmonicsCreator::levels(unsigned t0, unsigned t_delta, unsigned n0) {
    m_TData.clear();

    for (int n = 0; n < n0; n++) {
        double sum = 0;
        double t = t0 + n * t_delta;
        for (CoefficientsIterator it = m_Coefficients.constBegin(); it != m_Coefficients.constEnd(); ++it) {
            double w = 2 * M_PI * m_ModeFrequency[it.key()];
            sum += (it.value() * exp(m_I * w * t)).x;
        }
        m_TData.push_back(sum);
    }
    return m_TData;
}


