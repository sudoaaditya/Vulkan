
#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>

class Clock {

    private:
        using ClockType = std::chrono::steady_clock;
        ClockType::time_point startTime;
        ClockType::time_point lastTime;
        bool running;

    public:
        Clock();
        void start();
        double getElapsedTime();
        double getDeltaTime();
};

#endif // CLOCK_H