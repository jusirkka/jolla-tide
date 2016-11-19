#include "PatchIterator.h"
#include <QDebug>

using namespace Tide;


Patch::Patch(const Timestamp& start, const Interval& step, db_int_t size, const Epochs& epochs):
    m_Start(start),
    m_Step(step),
    m_Size(size),
    m_Epochs(epochs)
{}

Patch::Patch(const Timestamp& start, const Interval& step, db_int_t size, const Interval& offset):
    m_Start(start),
    m_Step(step),
    m_Size(size),
    m_Offset(offset)
{
}

Patch::Patch(const Patch& a):
    m_Start(a.start()),
    m_Step(a.step()),
    m_Size(a.size()),
    m_Epochs(a.epochs())
{}

Patch& Patch::operator=(const Patch& a) {
    m_Start = a.start();
    m_Step = a.step();
    m_Size = a.size();
    m_Offset = a.offset();
    m_Epochs = a.epochs();
    return *this;
}

Patch::Patch() {}

Patch Patch::offsetCopy(const Timestamp& epoch) {
    return Patch(m_Start, m_Step, m_Size, m_Start - epoch);
}

bool Patch::operator==(const Patch &other) const {
    if (m_Start != other.m_Start) return false;
    if (m_Step != other.m_Step) return false;
    if (m_Size != other.m_Size) return false;
    return m_Epochs == other.m_Epochs;
}

bool Patch::operator!=(const Patch &other) const {
    return !(*this == other);
}

const Timestamp& Patch::start() const {
    return m_Start;
}
const Interval& Patch::step() const {
    return m_Step;
}

const Interval& Patch::offset() const {
    return m_Offset;
}

db_int_t Patch::size() const {
    return m_Size;
}

const Patch::Epochs& Patch::epochs() const {
    return m_Epochs;
}


PatchIterator::PatchIterator(db_int_t station_id) {

    QList<QVector<QVariant>> r;
    QVariantList vars;
    vars << station_id;
    r = Database::Query("select id, start, timedelta, patchsize from epochs where station_id=? order by start", vars);


    db_int_t patch_start = -1;
    db_int_t patch_step = -1;
    db_int_t patch_last = -1;

    Patch::Epochs epochs;

    foreach (QVector<QVariant> row, r) {
        db_int_t epoch_id = row[0].toInt();
        db_int_t start = row[1].toInt();
        db_int_t step = row[2].toInt();
        db_int_t size = row[3].toInt();

        m_FirstStamp[epoch_id] = start;
        m_LastStamp[epoch_id] = start + step * (size - 1);
        m_Steps[epoch_id] = step;


        if (patch_start < 0) {
            patch_start = start;
            patch_step = step;
            patch_last = start + step * (size - 1);
            epochs.append(epoch_id);
            continue;
        }

        if (patch_last + 2 * patch_step < start) {
            Timestamp t = Timestamp::fromPosixTime(patch_start);
            Interval i = Interval::fromSeconds(patch_step);
            db_int_t s = (patch_last - patch_start) / patch_step + 1;
            qDebug() << "patch" << Timestamp::fromPosixTime(patch_start).print()
                     << Timestamp::fromPosixTime(patch_last).print()
                     << epochs
                     << s;
            m_Patches.append(Patch(t, i, s, epochs));
            patch_start = start;
            patch_step = step;
            patch_last = start + step * (size - 1);
            epochs.clear();
            epochs.append(epoch_id);
            continue;
        }

        if (start + step * (size - 1) < patch_last) {
            qDebug() << "skipping redundant epoch" << epoch_id;
            continue;
        }

        epochs.append(epoch_id);
        if (step > patch_step) patch_step = step;
        patch_last = start + step * (size - 1);
    }

    if (r.size() > 0) {
        // Append last patch
        Timestamp t = Timestamp::fromPosixTime(patch_start);
        Interval i = Interval::fromSeconds(patch_step);
        db_int_t s = (patch_last - patch_start) / patch_step + 1;
        qDebug() << "patch" << Timestamp::fromPosixTime(patch_start).print()
                 << Timestamp::fromPosixTime(patch_last).print()
                 << epochs
                 << s;
        m_Patches.append(Patch(t, i, s, epochs));
    }

    m_CurrentPatch = -1;
}

Patch PatchIterator::data() const {
    Q_ASSERT(m_CurrentPatch >= 0 && m_CurrentPatch < m_Patches.size());
    return m_Patches[m_CurrentPatch];
}


bool PatchIterator::nextPatch() {
    if (m_CurrentPatch + 1 >= m_Patches.size()) return false;
    m_CurrentPatch += 1;
    m_CurrentEpoch = 0;
    m_Step = data().step().seconds;
    m_CurrentStamp = m_FirstStamp[data().epochs().at(m_CurrentEpoch)] - m_Step;
    return true;
}

void PatchIterator::reset() {
    m_CurrentPatch = -1;
}

Timestamp PatchIterator::lastDataPoint() {
    Q_ASSERT(!m_Patches.isEmpty());
    Patch p = m_Patches.last();
    db_int_t epoch_id = p.epochs().last();
    return Timestamp::fromPosixTime(m_LastStamp[epoch_id]);
}

Timestamp PatchIterator::epoch() const {
    Q_ASSERT(!m_Patches.isEmpty());
    return m_Patches.first().start();
}


bool PatchIterator::next() {
    Patch::Epochs epochs = data().epochs();
    if (m_CurrentEpoch >= epochs.size()) return false;
    m_CurrentStamp += m_Step;
    if (m_CurrentStamp > m_LastStamp[epochs.at(m_CurrentEpoch)]) {
        m_CurrentEpoch += 1;
    }
    if (m_CurrentEpoch >= epochs.size()) return false;
    return true;
}

Timestamp PatchIterator::stamp() const {
    return Timestamp::fromPosixTime(m_CurrentStamp);
}


double PatchIterator::reading() {
    Patch::Epochs epochs = data().epochs();
    qint64 epoch_id = epochs[m_CurrentEpoch];

    if (m_Readings[epoch_id].isEmpty()) {
        getReadings();
    }

    if (m_CurrentStamp < m_FirstStamp[epoch_id]) {
        Q_ASSERT(m_CurrentEpoch > 0);
        qDebug() << "between patches: linear interpolation";
        db_int_t prev_id = epochs[m_CurrentEpoch - 1];
        double delta = m_CurrentStamp - m_LastStamp[prev_id];
        //  m_LastStamp[prev_id] < m_CurrentStamp < m_FirstStamp[epoch_id]
        Q_ASSERT(delta > 0);
        double x = delta / (m_FirstStamp[epoch_id] - m_LastStamp[prev_id]);
        return m_Readings[epoch_id].first() * (1 - x) + m_Readings[prev_id].last() * x;
    }

    qint64 delta = m_CurrentStamp - m_FirstStamp[epoch_id];
    qint64 step = m_Steps[epoch_id];
    qint64 index = delta / step;
    if (delta % step == 0) {
        return m_Readings[epoch_id][index];
    }

    // linear interpolation
    double x = double (delta % step) / step;
    return m_Readings[epoch_id][index] * (1 - x) + m_Readings[epoch_id][index + 1] * x;
}

void PatchIterator::getReadings() {
    Patch::Epochs epochs = data().epochs();
    qint64 epoch_id = epochs[m_CurrentEpoch];

    QVariantList vars;
    vars << epoch_id;
    QList<QVector<QVariant>> raw = Database::Query("select reading from readings where epoch_id=?", vars);
    foreach (QVector<QVariant> v, raw) {
        m_Readings[epoch_id].append(v[0].toDouble());
    }
}
