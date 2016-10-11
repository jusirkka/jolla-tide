#ifndef SKYCAL_H
#define SKYCAL_H

#include "TideEvent.h"
#include "Timestamp.h"
#include "Coordinates.h"

namespace Skycal {

void AddSunMoonEvents(const Tide::Timestamp& start,
                      const Tide::Timestamp& end,
                      const Tide::Coordinates& loc,
                      Tide::TideEvent::Organizer& org);
}

#endif
