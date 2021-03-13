#include "CalendarManager.h"

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

CalendarManager::CalendarManager() {
  // TODO Load events from filesystem ?

  // Init memory pool linked list
  itemsPool[0].prev = nullptr;
  for (int i = 1; i < CAPACITY; ++i) {
    itemsPool[i].prev = &itemsPool[i - 1];
    itemsPool[i - 1].next = &itemsPool[i];
  }
  itemsPool[CAPACITY - 1].next = nullptr;
}

/**
 * Inserts a new event if enough space
 * @param event The event to insert
 * @return true if the operation succeeded (enough space), false otherwise
 */
bool CalendarManager::addEvent(CalendarEvent& event) {
  if (mCount >= CAPACITY) {
    return false;
  }

  // Init event
  auto* item = mFreePoolHead;
  mFreePoolHead = mFreePoolHead->next;
  item->event = event;

  // Insert event
  auto* it = mEventsHead;
  // If list is empty
  if (it == nullptr) {
    item->prev = nullptr;
    item->next = nullptr;
    mEventsHead = item;
  }
  // Else if insert in first place
  else if (isBefore(event, it->event)) {
    item->prev = nullptr;
    item->next = mEventsHead;
    mEventsHead->prev = item;
    mEventsHead = item;
  } else {
    while (it->next != nullptr && !isBefore(event, it->event)) {
      it = it->next;
    }

    if (isBefore(event, it->event)) {
      item->prev = it->prev;
      item->next = it;
      it->prev->next = item;
      it->prev = item;
    } else {
      item->next = nullptr;
      item->prev = it;
      it->next = item;
    }
  }

  mCount++;
  return true;
}

/**
 * Removes the event corresponding to the given id if found
 * @param id The id of the event to remove
 * @return true if the event is found, false otherwise
 */
bool CalendarManager::deleteEvent(CalendarManager::CalendarEvent::Id id) {
  auto* item = mEventsHead;
  if (item == nullptr) {
    return false;
  }

  // Special case for deleting the first element
  if (item->event.id == id) {
    // Delete item from event list
    mEventsHead = item->next;
    if (mEventsHead != nullptr) {
      mEventsHead->prev = nullptr;
    }

    // Add item's memory back to free pool
    if (mFreePoolHead != nullptr) {
      mFreePoolHead->prev = item;
    }
    item->next = mFreePoolHead;
    mFreePoolHead = item;

    mCount--;
    return true;
  }

  // Searching for a matching event
  while (item != nullptr) {
    if (item->event.id == id) {
      // Delete item from event list
      item->prev->next = item->next;
      if (item->next != nullptr) {
        item->next->prev = item->prev;
      }

      // Add item's memory back to free pool
      if (mFreePoolHead != nullptr) {
        mFreePoolHead->prev = item;
      }
      item->next = mFreePoolHead;
      mFreePoolHead = item;

      mCount--;
      return true;
    }
    item = item->next;
  }

  return false;
}

CalendarManager::CalendarIterator CalendarManager::begin() {
  return CalendarIterator(mEventsHead);
}

CalendarManager::CalendarIterator CalendarManager::end() {
  return CalendarIterator(nullptr);
}

bool CalendarManager::empty() const {
  return mEventsHead == nullptr;
}

uint8_t CalendarManager::getCount() const {
  return mCount;
}
