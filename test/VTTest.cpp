#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../VTEvent.h"
#include "../VTStringUtil.h"
#include "../VTThreadPool.h"

#include <functional>


TEST_CASE("VTUtils/VTEvent/constructor", "Should construct not signaled event")
{
    VT::Event event;
    REQUIRE(event.is_signaled() == false);
}


TEST_CASE("VTUtils/VTEvent/signal", "Should set event's signaled state")
{
    VT::Event event;
    event.signal();
    REQUIRE(event.is_signaled() == true);
}


TEST_CASE("VTUtils/VTEvent/reset", "Should unset event's signaled state")
{
    VT::Event event;
    event.signal();
    event.reset();
    REQUIRE(event.wait(0) == false);
    REQUIRE(event.is_signaled() == false);
}


TEST_CASE("VTUtils/VTEvent/timeouts", "Verious timeouts")
{
    // timeouts are not too long to speed up testing
    VT::Event event;
    REQUIRE(event.wait(1) == false);
    event.signal();
    REQUIRE(event.wait(1) == true);
    REQUIRE(event.wait() == true);  // infinite wait
    event.reset();
    REQUIRE(event.wait(1) == false);
    REQUIRE(event.is_signaled() == false);
}


TEST_CASE("VTUtils/VTStringUtils/trim", "")
{
    CHECK(VT::StrUtils::trim("  Bus     ") == "Bus");
    CHECK(VT::StrUtils::trim("Bus     ") == "Bus");
    CHECK(VT::StrUtils::trim("    Bus") == "Bus");
    CHECK(VT::StrUtils::trim("Bus") == "Bus");
    CHECK(VT::StrUtils::trim("a Bus") == "a Bus");
}


TEST_CASE("VTUtils/VTStringUtils/split", "")
{
    std::vector<std::string> contspl;
    VT::StrUtils::split("one,two,three", &contspl, ",");
    REQUIRE(contspl.size() == 3);
    REQUIRE(contspl[0] == "one");
    REQUIRE(contspl[1] == "two");
    REQUIRE(contspl[2] == "three");
}


TEST_CASE("VTUtils/VTStringUtils/to_lower", "")
{
    REQUIRE(VT::StrUtils::to_lower("BaUojdUIOnsa adf DDF") == "bauojduionsa adf ddf");
}


TEST_CASE("VTUtils/VTStringUtils/empty", "Empty string")
{
    REQUIRE(VT::StrUtils::to_lower("") == "");
}


TEST_CASE("VTUtils/VTStringUtils/symbols", "Numbers and symbols")
{
    REQUIRE(VT::StrUtils::to_lower("1234567890!@#$%^&*()_+-=:';{}[]<>,./?~`") == "1234567890!@#$%^&*()_+-=:';{}[]<>,./?~`");
}


TEST_CASE("VTUtils/VTThreadPool/constructor", "")
{
    VT::ThreadPool tp(2, 5);
    REQUIRE(tp.thread_count() == 2);
    REQUIRE(tp.max_thread_count() == 5);
    REQUIRE(tp.wait_for_all() == true);
}


TEST_CASE("VTUtils/VTThreadPool/change_count", "")
{
    VT::ThreadPool tp(2, 5);
    tp.set_thread_count(3);
    REQUIRE(tp.thread_count() == 3);
    tp.set_thread_count(1);
    REQUIRE(tp.thread_count() == 1);
}


TEST_CASE("VTUtils/VTThreadPool/change_max_count", "")
{
    VT::ThreadPool tp(2, 5);
    tp.set_max_thread_count(7);
    REQUIRE(tp.max_thread_count() == 7);
    tp.set_max_thread_count(4);
    REQUIRE(tp.max_thread_count() == 4);
    tp.set_max_thread_count(1);
    REQUIRE(tp.max_thread_count() == 1);
    REQUIRE(tp.thread_count() == 1);
}


void work(int& param)
{
    param += 1;
}

TEST_CASE("VTUtils/VTThreadPool/run", "")
{
    VT::ThreadPool tp(2, 5);
    int x = 41;
    tp.run(std::bind(&work, std::ref(x)));
    tp.wait_for_all();
    REQUIRE(x == 42);
}


TEST_CASE("VTUtils/VTThreadPool/run_multiple", "")
{
    VT::ThreadPool tp(2, 5);
    std::vector<int> v(10);
    for (int i = 0; i < 10; ++i)
    {
        v[i] = 41;
        tp.run(std::bind(&work, std::ref(v[i])));
    }
    tp.wait_for_all();
    
    for (int i = 0; i < 10; ++i)
        REQUIRE(v[i] == 42);
}
