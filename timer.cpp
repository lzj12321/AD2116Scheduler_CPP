#include "timer.h"

Timer::Timer()
{
    connect(this,SIGNAL(timeout()),this,SLOT(outTime()));
}

void Timer::setTimerData(const uint &number, const uint &descriptor)
{
    timerNumber=number;
    timerDescriptor=descriptor;
}

void Timer::getTimerData(uint &number, uint &descriptor)
{
    number=timerNumber;
    descriptor=timerDescriptor;
}

uint Timer::getTimerDescriptor()
{
    return timerDescriptor;
}

void Timer::outTime()
{
    emit timerOutTime(timerNumber);
}
