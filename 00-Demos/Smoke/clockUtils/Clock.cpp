#include "Clock.h"

Clock::Clock() {
    running = false;
}

void Clock::start() {
    startTime = ClockType::now();
    lastTime = startTime;
    running = true;
}

double Clock::getElapsedTime() {
    if(!running) {
        return 0.0;
    }

    const ClockType::time_point currentTime = ClockType::now();
    return std::chrono::duration<double>(currentTime - startTime).count();
}

double Clock::getDeltaTime() {
    if(!running) {
        return 0.0;
    }

    const ClockType::time_point currentTime = ClockType::now();
    const double deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
    lastTime = currentTime;
    return deltaTime;
}
