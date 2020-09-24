#include "serverthread.h"

ServerThread::ServerThread()
{

}

void ServerThread::run()
{
//    connect(&ultraServer,SIGNAL(ultraReceivedMsg(QString)),this,SIGNAL(ultraReceivedMsg(QString)));
//    connect(&ultraServer,SIGNAL(ultraDisconnected()),this,SIGNAL(ultraDisconnected()));
//    connect(&ultraServer,SIGNAL(ultraConnected()),this,SIGNAL(ultraConnected()));
    ultraServer.runServer();
}
