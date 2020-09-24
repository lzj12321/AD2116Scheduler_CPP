#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H
#include<QThread>
#include <QObject>
#include<ultrasonicserver.h>

class ServerThread:public QThread
{
    Q_OBJECT
public:
    ServerThread();
    UltrasonicServer ultraServer;
protected:
    void run() override;
signals:
    void ultraReceivedMsg(QString);
    void ultraConnected();
    void ultraDisconnected();
};

#endif // SERVERTHREAD_H
