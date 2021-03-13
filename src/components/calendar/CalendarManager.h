#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <list>

namespace Pinetime {
  namespace Controllers {
    class CalendarManager {
    public:
      struct CalendarEvent {
        using Id = uint8_t;
        Id id;
        std::string title;
        uint32_t timestamp;
        uint32_t duration;
      };
      using CalendarEventIterator =  std::list<CalendarEvent>::iterator;

    private:
      static bool isBefore(CalendarEvent& event1, CalendarEvent& event2);

      std::list<CalendarEvent> calendarEvents;

    public:
      bool addEvent(CalendarEvent& event);
      bool deleteEvent(CalendarEvent::Id id);
      CalendarEventIterator begin();
      CalendarEventIterator end();
      bool empty() const;
      unsigned int getCount() const;
    };
  }
}