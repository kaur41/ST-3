// Copyright 2025 UNN-CS
#include "TimedDoor.h"
#include <thread>
#include <chrono>
#include <stdexcept>

// ============== DoorTimerAdapter ==============
DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
    door.throwState();
}

// ============== TimedDoor ==============
TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false) {
    adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
    delete adapter;
}

bool TimedDoor::isDoorOpened() {
    return isOpened;
}

void TimedDoor::unlock() {
    isOpened = true;
    Timer timer;
    timer.tregister(iTimeout, adapter);
}

void TimedDoor::lock() {
    isOpened = false;
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    if (isOpened) {
        throw std::runtime_error("Door is still opened after timeout!");
    }
}

// ============== Timer ==============
void Timer::sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Timer::tregister(int timeout, TimerClient* client) {
    // Запускаем таймер в отдельном потоке
    std::thread([timeout, client]() {
        std::this_thread::sleep_for(std::chrono::seconds(timeout));
        client->Timeout();
        }).detach();
}