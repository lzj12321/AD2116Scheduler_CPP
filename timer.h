#ifndef TIMER_H
#define TIMER_H
#include<QTimer>

class Timer:public QTimer
{
    Q_OBJECT
public:
    Timer();
    void setTimerData(const uint& number,const uint& descriptor);
    void getTimerData(uint&,uint&);
    uint getTimerDescriptor();
private:
    uint timerNumber;
    uint timerDescriptor;
    uint currentState;
    uint nextState;
signals:
    void timerOutTime(uint);
private slots:
    void outTime();
};

#endif // TIMER_H
