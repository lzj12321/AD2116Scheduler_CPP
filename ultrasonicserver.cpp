#include "ultrasonicserver.h"
#include<QDebug>
#include<QStringList>
#include<unistd.h>

UltrasonicServer::UltrasonicServer()
{
}

UltrasonicServer::~UltrasonicServer()
{
    if(clientSocket==-1)
        close(clientSocket);
    if(serverSock==-1)
        close(serverSock);
}

bool UltrasonicServer::runServer(){
    //****创建套接字
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    //Windows中，AF_INET==PF_INET
    //Linux中，不同的版本这两者有微小差别.对于BSD是AF,对于POSIX是PF
    if (serverSock < 0)
    {
        emit ultraServerIniError();
        return false;
    }
    //****绑定ip和端口
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(USEPORT);
    //INADDR_ANY绑定所有IP
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //****绑定套接字
    if (bind(serverSock,
             (struct sockaddr*)&serverAddr,
             sizeof(struct sockaddr)) == -1)
    {
        serverSock=-1;
        emit ultraServerIniError();
        return false;
    }

    //****监听
    if (listen(serverSock, 10) == -1)
    {
        emit ultraServerIniError();
        return false;
    }

    //****开始接收accept()
    struct sockaddr clientAddr;
    int size = sizeof(struct sockaddr);
    while (true)
    {
        if((clientSocket = accept(serverSock, (struct sockaddr*)&clientAddr, (socklen_t*)&size))==-1){
            clientSocket=-1;
            continue;
        }
        emit ultraConnected();
        while (true) {
            QString msg=receivedMsg();
            if(msg.length()==0){
                ++offlineTime;
                if(offlineTime>1000){
                    close(clientSocket);
                    clientSocket=-1;
                    offlineTime=0;
                    emit ultraDisconnected();
                    break;
                }
            }else{
                sleep(0.05);
                offlineTime=0;
                QStringList msgList=msg.split('\n');
                for(uint i=0;i<msgList.size();++i){
                    if(msgList[i].remove("\n").isEmpty())
                        continue;
                    emit ultraReceivedMsg(msgList[i]);
                }
            }
        }
    }

    return true;
}

QString UltrasonicServer::receivedMsg(){
    int n=recv(clientSocket,buff,1024,0);
    buff[n]='\0';
    //    qDebug()<<n<<endl;
    return QString(buff);
}

bool UltrasonicServer::sendMsg(QString msg){
    send(clientSocket, msg.toStdString().c_str(), msg.length(), 0);
    return true;
}
