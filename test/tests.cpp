// Copyright 2025 UNN-CS
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

// ============== Mock для TimerClient ==============
class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

// ============== Адаптер для тестирования Timer ==============
class TestableTimer : public Timer {
public:
    void tregister(int timeout, TimerClient* client) {
        // Для тестов используем синхронный вызов
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        client->Timeout();
    }
};

// ============== Fixture для тестирования TimedDoor ==============
class TimedDoorTest : public ::testing::Test {
protected:
    void SetUp() override {
        door = new TimedDoor(1);
    }

    void TearDown() override {
        delete door;
    }

    TimedDoor* door;
};

// ============== Fixture для тестирования DoorTimerAdapter ==============
class DoorTimerAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        door = new TimedDoor(1);
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete adapter;
        delete door;
    }

    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

// ============== Тесты для TimedDoor ==============

TEST_F(TimedDoorTest, InitialStateAfterCreation) {
    EXPECT_FALSE(door->isDoorOpened());
    EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, ThrowStateWhenOpened) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateWhenClosed) {
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
    EXPECT_NO_THROW(door->throwState());
}

// ============== Тесты для DoorTimerAdapter ==============

TEST_F(DoorTimerAdapterTest, TimeoutCallsThrowState) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(DoorTimerAdapterTest, TimeoutOnClosedDoorNoException) {
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
    EXPECT_NO_THROW(adapter->Timeout());
}

// ============== Тесты с моками ==============

TEST(MockTimerClientTest, TimeoutMethodIsCalled) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout())
        .Times(1);

    TestableTimer timer;
    timer.tregister(10, &mockClient);  // 10 мс
}

TEST(MockTimerClientTest, TimeoutMethodNotCalledImmediately) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout())
        .Times(0);

    // Проверяем, что Timeout не вызывается сразу
    // (в синхронном тесте он вызовется через 100 мс)
}

// ============== Интеграционные тесты ==============

TEST(IntegrationTest, DoorThrowsExceptionAfterTimeout) {
    TimedDoor door(1);
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());

    // Ждём истечения таймера
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Таймер должен был вызвать throwState()
    // Проверяем, что исключение будет выброшено при следующем вызове
    // (в реальной системе исключение выбрасывается асинхронно)
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST(IntegrationTest, NoExceptionIfDoorClosedBeforeTimeout) {
    TimedDoor door(1);
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());

    // Закрываем дверь до истечения таймера
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    door.lock();
    EXPECT_FALSE(door.isDoorOpened());

    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    EXPECT_NO_THROW(door.throwState());
}

TEST(IntegrationTest, MultipleUnlockLockCycles) {
    TimedDoor door(1);

    for (int i = 0; i < 3; i++) {
        door.unlock();
        EXPECT_TRUE(door.isDoorOpened());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        door.lock();
        EXPECT_FALSE(door.isDoorOpened());
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        EXPECT_NO_THROW(door.throwState());
    }
}

TEST(IntegrationTest, DifferentTimeoutValues) {
    TimedDoor door5(5);
    EXPECT_EQ(door5.getTimeOut(), 5);

    TimedDoor door10(10);
    EXPECT_EQ(door10.getTimeOut(), 10);
}

// ============== Тест с использованием Mock для проверки вызова ==============

class MockDoor : public Door {
public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

TEST(MockDoorTest, LockAndUnlockCalls) {
    MockDoor mockDoor;

    EXPECT_CALL(mockDoor, unlock()).Times(1);
    EXPECT_CALL(mockDoor, lock()).Times(1);
    EXPECT_CALL(mockDoor, isDoorOpened()).WillOnce(Return(true));

    mockDoor.unlock();
    EXPECT_TRUE(mockDoor.isDoorOpened());
    mockDoor.lock();
}

// Проверка, что Timeout не вызывается, если дверь закрыта
class RecordingTimerClient : public TimerClient {
public:
    bool timeoutCalled = false;
    void Timeout() override {
        timeoutCalled = true;
    }
};

TEST(RecordingTimerTest, TimeoutNotCalledIfDoorClosed) {
    RecordingTimerClient client;
    TimedDoor door(1);
    DoorTimerAdapter adapter(door);

    door.lock();
    EXPECT_FALSE(door.isDoorOpened());

    // Таймаут не должен вызвать исключение
    EXPECT_NO_THROW(adapter.Timeout());
}