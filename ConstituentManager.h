#ifndef CONSTITUENT_MANAGER_H
#define CONSTITUENT_MANAGER_H

#include <QMap>
#include <QVector>

#include "Timestamp.h"
#include "Speed.h"
#include "Complex.h"
#include "PatchIterator.h"

namespace Tide {


class ConstituentManager {

public:

    typedef QMap<Speed, Complex> Coefficients;

    ConstituentManager(int stationId);
    bool next();
    Patch patch() const;
    Timestamp epoch() const;
    Coefficients constituents();

    ~ConstituentManager() {}

private:

    void deleteConstituents();
    void createConstituent();

private:

    int m_Station;
    PatchIterator m_Patches;
    QVector<int> m_PatchIds;
    QMap<int, Patch> m_PatchData;
};



}

#endif //
