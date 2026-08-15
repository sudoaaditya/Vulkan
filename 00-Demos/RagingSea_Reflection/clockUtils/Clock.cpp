#include "Clock.h"
using namespace std;


Clock::Clock() {
    running = false;
}

void Clock::start() {
    startTime = clock();
    lastTime = startTime;
    running = true;
}

double Clock::getElapsedTime() {
    if(!running) return 0.0;

    return (clock() - startTime) / (double)CLOCKS_PER_SEC;
}

double Clock::getDeltaTime() {
    if(!running) return 0.0;
    
    clock_t currentTime = clock();
    double deltaTime = (currentTime - lastTime) / (double)CLOCKS_PER_SEC;
    lastTime = currentTime;
    return deltaTime;
}
