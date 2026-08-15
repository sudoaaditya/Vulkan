
#ifndef CLOCK_H
#define CLOCK_H

#include<ctime>

class Clock {

    private:
        std::clock_t startTime;
        std::clock_t lastTime;
        bool running;

    public:
        Clock();
        void start();
        double getElapsedTime();
        double getDeltaTime();
};

#endif CLOCK_H