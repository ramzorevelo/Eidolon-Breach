/**
 * @file test_EventBus.cpp
 * @brief Tests for EventBus subscription, emission, and scope cleanup.
 */
#include "Core/EventBus.h"
#include "doctest.h"
#include <string>

namespace
{
struct TestEvent
{
    int value{0};
};
struct OtherEvent
{
    std::string label{};
};
} // namespace

TEST_CASE("EventBus: emit calls subscriber with correct event data")
{
    EventBus bus{};
    int received{-1};
    bus.subscribe<TestEvent>([&received](const TestEvent &e)
                             { received = e.value; });
    bus.emit(TestEvent{42});
    CHECK(received == 42);
}

TEST_CASE("EventBus: multiple subscribers all fire in subscription order")
{
    EventBus bus{};
    std::vector<int> order{};
    bus.subscribe<TestEvent>([&order](const TestEvent &)
                             { order.push_back(1); });
    bus.subscribe<TestEvent>([&order](const TestEvent &)
                             { order.push_back(2); });
    bus.emit(TestEvent{0});
    REQUIRE(order.size() == 2);
    CHECK(order[0] == 1);
    CHECK(order[1] == 2);
}

TEST_CASE("EventBus: emit with no subscribers does not crash")
{
    EventBus bus{};
    bus.emit(TestEvent{99}); // must not throw
}

TEST_CASE("EventBus: different event types do not cross-fire")
{
    EventBus bus{};
    bool testFired{false};
    bool otherFired{false};
    bus.subscribe<TestEvent>([&testFired](const TestEvent &)
                             { testFired = true; });
    bus.subscribe<OtherEvent>([&otherFired](const OtherEvent &)
                              { otherFired = true; });
    bus.emit(TestEvent{0});
    CHECK(testFired);
    CHECK(!otherFired);
}

TEST_CASE("EventBus: clearBattleScope removes Battle-scoped handlers")
{
    EventBus bus{};
    int count{0};
    bus.subscribe<TestEvent>([&count](const TestEvent &)
                             { ++count; }, EventScope::Battle);
    bus.emit(TestEvent{0});
    CHECK(count == 1);
    bus.clearBattleScope();
    bus.emit(TestEvent{0});
    CHECK(count == 1); // not called again
}

TEST_CASE("EventBus: clearBattleScope preserves Run-scoped handlers")
{
    EventBus bus{};
    int count{0};
    bus.subscribe<TestEvent>([&count](const TestEvent &)
                             { ++count; }, EventScope::Run);
    bus.clearBattleScope();
    bus.emit(TestEvent{0});
    CHECK(count == 1); // still fires
}

TEST_CASE("EventBus: clearRunScope removes Run-scoped handlers")
{
    EventBus bus{};
    int count{0};
    bus.subscribe<TestEvent>([&count](const TestEvent &)
                             { ++count; }, EventScope::Run);
    bus.clearRunScope();
    bus.emit(TestEvent{0});
    CHECK(count == 0);
}

TEST_CASE("EventBus: clear on empty bus does not crash")
{
    EventBus bus{};
    bus.clearBattleScope();
    bus.clearRunScope();
}