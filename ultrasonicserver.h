#ifndef ULTRASONICSERVER_H
#define ULTRASONICSERVER_H
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<QString>
using namespace std;
#define USEPORT 6666
#include <QObject>

class UltrasonicServer:public QObject
{
    Q_OBJECT
public:
    UltrasonicServer();
    ~UltrasonicServer();
    bool runServer();
    QString receivedMsg();
    bool sendMsg(QString);

private:
    int serverSock=-1;
    int clientSocket=-1;
    char buff[1024];
    int offlineTime=0;

signals:
    void ultraReceivedMsg(QString);
    void ultraConnected();
    void ultraDisconnected();
    void ultraServerIniError();
};

#endif // ULTRASONICSERVER_H
