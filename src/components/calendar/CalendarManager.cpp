#include "CalendarManager.h"
#include <algorithm>

using namespace Pinetime::Controllers;

/**
 * Determines if the event `event1` should be placed before event `event2`
 * @param event1 The event to be inserted
 * @param event2 The comparison event
 * @return
 */
bool CalendarManager::isBefore(CalendarEvent& event1, CalendarEvent& event2) {
  return event1.timestamp < event2.timestamp;
}


/**
 * Inserts a new event if enough space
 * @param event The event to insert
 * @return true if the operation succeeded (enough space), false otherwise
 */
bool CalendarManager::addEvent(CalendarEvent& event) {
  auto it = calendarEvents.begin();

  while (it != calendarEvents.end() && isBefore(*it, event)) {
   ++it;
  }
  calendarEvents.insert(it, event);
  return true;
}

/**
 * Removes the event corresponding to the given id if found
 * @param id The id of the event to remove
 * @return true if the event is found, false otherwise
 */
bool CalendarManager::deleteEvent(CalendarManager::CalendarEvent::Id id) {
  auto matchId = [id](const CalendarEvent& event) { return event.id == id; };
  auto it = std::find_if(calendarEvents.begin(), calendarEvents.end(), matchId);

  if (it != calendarEvents.end()) {
    calendarEvents.erase(it);
    return true;
  }

  return false;
}

CalendarManager::CalendarEventIterator CalendarManager::begin() {
  return calendarEvents.begin();
}

CalendarManager::CalendarEventIterator CalendarManager::end() {
  return calendarEvents.end();
}

bool CalendarManager::empty() const {
  return calendarEvents.empty();
}

unsigned int CalendarManager::getCount() const {
  return calendarEvents.size();
}
