#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>

namespace Pinetime {
  namespace Controllers {
    class CalendarManager {
    public:
      static constexpr uint8_t CAPACITY = 15;
      static constexpr uint8_t TITLE_SIZE {100};
      struct CalendarEvent {
        using Id = uint8_t;
        Id id;
        std::string title;
        uint32_t timestamp;
        uint32_t duration;
      };

    private:
      static bool isBefore(CalendarEvent& event1, CalendarEvent& event2);
      struct CalendarItem {
        CalendarEvent event;
        CalendarItem* prev;
        CalendarItem* next;
      };
      uint8_t mCount = 0;
      CalendarItem itemsPool[CAPACITY] {};
      CalendarItem* mEventsHead = nullptr;
      CalendarItem* mFreePoolHead = &itemsPool[0];

    public:
      class CalendarIterator : public std::iterator<std::bidirectional_iterator_tag, CalendarEvent> {
        friend class CalendarManager;
        CalendarIterator(CalendarItem* i) : item(i) {
        }

      public:
        CalendarIterator() : item(nullptr) {
        }

        CalendarIterator(const CalendarIterator& i) : item(i.item) {
        }

        CalendarIterator& operator--() {
          item = item->prev;
          return *this;
        }

        CalendarIterator operator--(int) {
          auto tmp {*this};
          operator--();
          return tmp;
        }

        CalendarIterator& operator++() {
          item = item->next;
          return *this;
        }

        CalendarIterator operator++(int) {
          auto tmp {*this};
          operator++();
          return tmp;
        }

        CalendarIterator& operator=(const CalendarIterator& i) {
          item = i.item;
          return *this;
        }

        bool operator==(const CalendarIterator& i) {
          return item == i.item;
        }

        bool operator!=(const CalendarIterator& i) {
          return item != i.item;
        }

        CalendarEvent const& operator*() {
          return item->event;
        }

      private:
        CalendarItem* item;
      };

      CalendarManager();
      bool addEvent(CalendarEvent& event);
      bool deleteEvent(CalendarEvent::Id id);
      CalendarIterator begin();
      static CalendarIterator end();
      bool empty() const;
      uint8_t getCount() const;
    };
  }
}