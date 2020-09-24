#include "socket.h"

Socket::Socket()
{

}

void Socket::setSocketFlag(const uint descriptor)
{
    socketDescriptor=descriptor;
}

void Socket::setSocket(QTcpSocket * sockPtr)
{
    socket=sockPtr;
    connect(sockPtr,SIGNAL(readyRead()),this,SLOT(receiveMsg()));
    connect(sockPtr,SIGNAL(disconnected()),this,SLOT(disconnectFromServer()));
}

const uint Socket::getSocketFlag()
{
    return socketDescriptor;
}

bool Socket::readMsg(QString &msg)
{
    uint maxSize=20;
    char data[40];
    if(socket->readLine(data,maxSize)==-1)
        return false;
    else{
        msg=QString(data);
        msg.remove("\n");
        return true;
    }
}

bool Socket::readMsg(std::vector<QString> &msgArray)
{
    while(true)
    {
        uint maxSize=40;
        char data[40];
        if(socket->readLine(data,maxSize)!=-1){
            QString msg=QString(data);
            msg.remove("\n");
            msgArray.push_back(msg);
        }
        else
            break;
    }
    if(msgArray.size()>0)
        return true;
    else
        return false;
}

bool Socket::sendMsg(const QString msg)
{
    if(!socket->isOpen()||!socket->isValid())
        return false;
    if(socket->write(msg.toStdString().c_str())==-1)
        return false;
    else
        return true;
}

void Socket::receiveMsg()
{
    emit readyToRead(socketDescriptor);
}

void Socket::disconnectFromServer()
{
    emit disconnectServer(socketDescriptor);
}
