#ifndef PATCHITERATOR_H
#define PATCHITERATOR_H

#include <QVector>

#include "Timestamp.h"
#include "Database.h"

namespace Tide {


class Patch {
public:

    typedef QVector<db_int_t> Epochs;

    Patch(const Timestamp& start, const Interval& step, db_int_t size, const Epochs& epochs);
    Patch(const Timestamp& start, const Interval& step, db_int_t size, const Interval& offset);
    Patch(const Patch& a);
    Patch& operator=(const Patch& a);
    Patch();

    Patch offsetCopy(const Timestamp& epoch);

    bool operator==(const Patch &other) const;
    bool operator!=(const Patch &other) const;

    const Timestamp& start() const;
    const Interval& step() const;
    const Interval& offset() const;
    db_int_t size() const;
    const Epochs& epochs() const;

    ~Patch() {}

private:

    Timestamp m_Start;
    Interval m_Step;
    db_int_t m_Size;
    Interval m_Offset;
    Epochs m_Epochs;

};

typedef QVector<Patch> PatchData;

class PatchIterator {

public:
    PatchIterator(db_int_t station_id);
    Patch data() const;
    bool next();
    bool nextPatch();
    Timestamp lastDataPoint();
    Timestamp epoch() const;
    double reading();

    ~PatchIterator() {}

private:

    void getReadings();

private:

    int m_CurrentPatch;
    db_int_t m_CurrentStamp;
    db_int_t m_Step;
    int m_CurrentEpoch;

    PatchData m_Patches;

    QMap<db_int_t, db_int_t> m_FirstStamp;
    QMap<db_int_t, db_int_t> m_LastStamp;
    QMap<db_int_t, db_int_t> m_Steps;
    QMap<db_int_t, QVector<double>> m_Readings;


};



}

#endif //
