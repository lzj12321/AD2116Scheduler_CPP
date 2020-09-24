#include "schedulergui.h"
#include "ui_schedulergui.h"
#include<QTime>
#include<QDateTime>
#include<QLineEdit>
#include<QPixmap>
#include<QSettings>
#include<QMessageBox>
#include<QInputDialog>
#include<QProcess>
#include<QDebug>


SchedulerGUI::SchedulerGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SchedulerGUI)
{
    ui->setupUi(this);
    paramIni();

    dataIni();

    serverIni();

    IoIni();

    deviceIni();

    uiIni();

    ultrasonicServerIni();

    connectSlotAndSignal();
    checkCatchableTimer.setInterval(checkCatchTimerInterval);
    checkCatchableTimer.start();
}

SchedulerGUI::~SchedulerGUI()
{
//    delete ultraServer;
    ultraThread->exit(0);
    delete ultraThread;

    slotReleaseAllPlate();
    stopConveyor();

    delete[] robotWorkStateArray;
    delete[] moduleStateArray;
    delete[] robotLabelArr;
    delete[] robotUnrecognizeNum;
    delete[] caughtNum;
    delete[] timerPtr;

    for(uint i=0;i<moduleNum;++i)
        delete moduleLabelArray[i];
    delete moduleLabelArray;

    delete[] robotUnrecognizeNumLineEdit;

    delete server;
    delete ui;
}

void SchedulerGUI::closeEvent(QCloseEvent *event)
{
    int choose;
    choose=QMessageBox::question(this,"Quit","Confirm quit the Scheduler?", QMessageBox::Yes | QMessageBox::No);
    if (choose==QMessageBox::No)
    {
        event->ignore();
    }
    else if(choose==QMessageBox::Yes)
    {
        for(uint i=0;i<robotNum;++i)
        {
            if(robotWorkStateArray[i]==ROBOT_ACK_CATCHED)
            {
                int isClose=QMessageBox::question(this,"Warning","There's a robot in catching status,are you sure quit the Scheduler?",QMessageBox::Yes|QMessageBox::No);
                if(isClose==QMessageBox::Yes)
                {
                    slotReleaseAllPlate();
                    stopConveyor();
                    event->accept();
                    break;
                }
                else
                    event->ignore();
            }
        }
    }
}

void SchedulerGUI::uiIni()
{
    addRunMessage("UI initialize!",Qt::green);

    ui->lineEdit_6->setText(QString::number(robotUnrecognizeNum[0],10));
    ui->lineEdit_7->setText(QString::number(robotUnrecognizeNum[1],10));
    ui->lineEdit_8->setText(QString::number(robotUnrecognizeNum[2],10));
    ui->lineEdit_12->setText(QString::number(robotUnrecognizeNum[3],10));

    ui->lineEdit_11->setText(QString::number(caughtNum[0],10));
    ui->lineEdit_9->setText(QString::number(caughtNum[1],10));
    ui->lineEdit_10->setText(QString::number(caughtNum[2],10));
    ui->lineEdit_13->setText(QString::number(caughtNum[3],10));

    QPixmap pixmapForward("forward.png");
    QPixmap pixmapReverse("reverse.png");
    //    pixmapForward
    ui->label_8->setScaledContents(true);
    ui->label_9->setScaledContents(true);
    ui->label_8->setPixmap(pixmapForward);
    ui->label_9->setPixmap(pixmapReverse);

    if(isProductiveTwoModel)
    {
        ui->radioButton->setChecked(true);
    }
    else
    {
        ui->radioButton_2->setChecked(true);
    }
    //    setRobotLabelPosition();
    iniModuleLabel();
    //    qDebug()<<"iniModuleLabel";
    updateUI();
    //        qDebug()<<"updateUI";
    //    showFullScreen();

    ui->radioButton->setVisible(false);
    ui->radioButton_2->setVisible(false);

    ui->checkBox->setChecked(isUltraValid);
    ui->checkBox_2->setChecked(isStopLastRobot);
}

void SchedulerGUI::dataIni()
{
    robotUnrecognizeNumLineEdit=new QLineEdit*[robotNum];
    robotCaughtNumLineEdit=new QLineEdit*[robotNum];

    robotUnrecognizeNumLineEdit[0]=ui->lineEdit_6;
    robotUnrecognizeNumLineEdit[1]=ui->lineEdit_7;
    robotUnrecognizeNumLineEdit[2]=ui->lineEdit_8;
    robotUnrecognizeNumLineEdit[3]=ui->lineEdit_12;

    robotCaughtNumLineEdit[0]=ui->lineEdit_11;
    robotCaughtNumLineEdit[1]=ui->lineEdit_9;
    robotCaughtNumLineEdit[2]=ui->lineEdit_10;
    robotCaughtNumLineEdit[3]=ui->lineEdit_13;

    for(uint i=0;i<robotNum;++i)
    {
        robotUnrecognizeNum[i]=0;
        caughtNum[i]=0;
    }
}

void SchedulerGUI::paramIni()
{
    //////////data and param initialize////////////
    addRunMessage("param initialize!",Qt::green);
    QString configFilePath="settings.ini";
    QFile file(configFilePath);
    if(!file.exists())
    {
        QMessageBox::warning(NULL,"Warning","配置文件丢失");
        exit(0);
    }

    QSettings settings(configFilePath,QSettings::IniFormat);
    isProductiveTwoModel=settings.value("runData/isProductiveTwoModel").toBool();
    _isProductiveTwoModel=isProductiveTwoModel;
    conveyorStatusIO=settings.value("io/conveyorStatusIO").toUInt();
    conveyorDirectionIO=settings.value("io/conveyorDirectionIO").toUInt();
    alarmStatusIO=settings.value("io/alarmStatusIO").toUInt();
    resetAlarmButtonIO=settings.value("io/resetAlarmButtonIO").toUInt();

    detectSensorPosition=settings.value("sensorPosition/detectSensorPosition").toUInt();
    //    qDebug()<<"detectSensorPosition:"<<detectSensorPosition;

    catchOrder=settings.value("order/catchOrder").toString();
    clearOrder=settings.value("order/clearOrder").toString();
    dropOrder=settings.value("order/dropOrder").toString();
    moduleNum=settings.value("deviceNumber/moduleNumber").toUInt();
    robotNum=settings.value("deviceNumber/robotNumber").toUInt();

    robotUnrecognizeNum=new uint[robotNum];
    caughtNum=new uint[robotNum];

    checkCatchTimerInterval=settings.value("detectParam/checkCatchTimerInterval").toUInt();
    maxDetectAdapterTime=settings.value("detectParam/maxDetectAdapterTime").toUInt();
    checkCatchingMsgInterval=settings.value("detectParam/checkCatchingMsgInterval").toUInt();
    checkCatchedMsgInterval=settings.value("detectParam/checkCatchedMsgInterval").toUInt();
    //    maxDetectModuleTime=settings.value("detectParam/maxDetectModuleTime").toUInt();
    checkResetButtonInterval=settings.value("detectParam/checkResetButtonInterval").toUInt();
    flashAlarmInterval=settings.value("detectParam/flashAlarmInterval").toUInt();

    modelGap=settings.value("uiParam/modelGap").toUInt();
    modelRadius=settings.value("uiParam/modelRadius").toUInt();
    labelGap=settings.value("uiParam/labelGap").toUInt();
    robotLabelWidth=settings.value("uiParam/robotLabelWidth").toUInt();
    robotLabelHeight=settings.value("uiParam/robotLabelHeight").toUInt();

    for(uint i=0;i<robotNum;++i){
        //        qDebug()<<i<<" ";
        QString _robotName=settings.value("robotName/robot"+QString::number(i,10)).toString();
        if(i!=ultrasonicIndex)
            robotName.insert(std::pair<uint,QString>(i,"机械手"+_robotName));
        else
            robotName.insert(std::pair<uint,QString>(i,_robotName));
        //        robotName.insert(std::pair<uint,QString>(i,_robotName));

        uint _robotModuleIO=settings.value("io/moduleSensor"+QString::number(i,10)).toUInt();
        robotModuleSensorIO.insert(std::pair<uint,uint>(i,_robotModuleIO));

        uint _robotPlateIO=settings.value("io/plate"+QString::number(i,10)).toUInt();
        robotPlateIO.insert(std::pair<uint,uint>(i,_robotPlateIO));

        uint _robotAdapterIO=settings.value("io/adapterSensor"+QString::number(i,10)).toUInt();
        robotAdapterSensorIO.insert(std::pair<uint,uint>(i,_robotAdapterIO));

        uint _robotModuleNumber=settings.value("robotModuleNumber/robot"+QString::number(i,10)).toUInt();
        robotModuleNumber.insert(std::pair<uint,uint>(i,_robotModuleNumber));

        QString _robotIP=settings.value("robotIP/robot"+QString::number(i,10)).toString();
        robotIP.insert(std::pair<uint,QString>(i,_robotIP));
    }

    robotMsgToStatusArray.insert(std::pair<QString,uint>("wait",ROBOT_WAITING));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("catched",ROBOT_CATCHED));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("unrecognize",ROBOT_RECOGNIZE_ERROR));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("clear",ROBOT_CLEARING));


    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur2_insert_error",ULTRASONIC_ROBOT2_INSERT_ERROR));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur1_catch_error",ULTRASONIC_ROBOT1_CATCH_ERROR));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur1_unrecognized",ULTRASONIC_ROBOT1_UNRECOGNIZED));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur2_unrecognized",ULTRASONIC_ROBOT2_UNRECOGNIZED));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur1_caught",ROBOT_CATCHED));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur2_inserted",ULTRASONIC_ROBOT2_INSERTED));
//    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur2_inserted-wait",ULTRASONIC_ROBOT2_INSERTED_WAIT));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur2_inserted-wait",ULTRASONIC_ROBOT2_INSERTED_WAIT));
    robotMsgToStatusArray.insert(std::pair<QString,uint>("ur1_caught-wait",ULTRASONIC_ROBOT2_CAUGHT_WAIT));
    ////////////ini robot status////////////
    ioUI=new IO_Control(this);

    moduleLabelArray=new QLabel*[moduleNum];
    for(uint i=0;i<moduleNum;++i)
    {
        moduleLabelArray[i]=new QLabel(this);
    }

    moduleStateArray=new uint[moduleNum];
    robotWorkStateArray=new uint[robotNum];

    robotLabelArr=new QLabel*[robotNum];
    robotLabelArr[0]=ui->label;
    robotLabelArr[1]=ui->label_2;
    robotLabelArr[2]=ui->label_3;
    robotLabelArr[3]=ui->label_4;
    conveyorLabel=ui->label_5;


    for(uint i=0;i<robotNum;++i)
        robotWorkStateArray[i]=ROBOT_OFFLINE;

    for(uint i=0;i<moduleNum;++i){
        moduleStateArray[i]=MODULE_EMPTY;
    }

    timerPtr=new Timer[robotNum];
    for(uint i=0;i<robotNum;++i){
        timerPtr[i].setTimerData(i,0);
        timerPtr[i].setInterval(checkCatchingMsgInterval);
        robotMsgTimer.insert(std::pair<uint,Timer*>(i,&timerPtr[i]));
    }
}

bool SchedulerGUI::IoIni()
{
    ///////////initialize the wiringpi io///////////
    if(gpio_pi.gpioIni())
    {
        addRunMessage("io initialize success!",Qt::green);
        return true;
    }
    else
    {
        addRunMessage("io initialize fail!",Qt::red);
        return false;
    }
}

void SchedulerGUI::deviceIni()
{
    ////////release all plate and stop the conveyor//////////
    stopConveyor();
}

void SchedulerGUI::updateUI()
{
    updateRobotUI();
    updateModuleUI();
}

void SchedulerGUI::updateModuleUI()
{
    for(uint i=0;i<moduleNum;++i){
        updateSingleModule(i,moduleStateArray[i]);
    }
}

void SchedulerGUI::updateSingleModule(uint moduleIndex, uint state)
{
    switch (state)
    {
    case MODULE_EMPTY:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:gray");
    }
        break;
    case MODULE_LOADED:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:blue");
    }
        break;
    case MODULE_CATCHING:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:yellow");
    }
        break;
    case MODULE_UNRECOGNIZE_ONCETIME:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:magenta");
    }
        break;
    case MODULE_UNRECOGNIZE_TWICETIME:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:darkMagenta");
    }
        break;
    case MODULE_ERROR:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:red");
    }
        break;
    case MODULE_ULTRASONIC:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:green");
    }
        break;
    case MODULE_ULTRA1_UNRECOGNIZED:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:black");
    }
        break;
    case MODULE_ULTRA2_UNRECOGNIZED:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background:red");
    }
        break;
    case MODULE_ULTRA1_CAUGHT:
    {
        moduleLabelArray[moduleIndex]->setStyleSheet("background-color:rgb(255, 119, 28)");
    }
        break;
    default:
        break;
    }
}

void SchedulerGUI::updateSingleRobot(uint robotIndex, uint state)
{
    switch (state){
    case ROBOT_CONNECTED:
    {
        robotLabelArr[robotIndex]->setText("CONNECTED");
        robotLabelArr[robotIndex]->setStyleSheet("background:blue");
    }
        break;
    case ROBOT_WAITING:
    {
        robotLabelArr[robotIndex]->setText("WAITTING");
        robotLabelArr[robotIndex]->setStyleSheet("background:yellow");
    }
        break;
    case ROBOT_CATCHED:
    {
        robotLabelArr[robotIndex]->setText("WORKING");
        robotLabelArr[robotIndex]->setStyleSheet("background:green");
    }
        break;
    case ROBOT_ACK_CATCHED:
    {
        robotLabelArr[robotIndex]->setText("WAIT CAUGHT");
        robotLabelArr[robotIndex]->setStyleSheet("background:green");
    }
        break;
    case ROBOT_OFFLINE:
    {
        robotLabelArr[robotIndex]->setText("OFFLINE");
        robotLabelArr[robotIndex]->setStyleSheet("background:red");
    }
        break;
    case ROBOT_ACK_CATCHED_ERROR:
    {
        robotLabelArr[robotIndex]->setText("ACK_CATCH_ERROR");
        robotLabelArr[robotIndex]->setStyleSheet("background:red");
    }
        break;
    case ROBOT_RECOGNIZE_ERROR:
    {
        robotLabelArr[robotIndex]->setText("RECOGNIZE ERROR");
        robotLabelArr[robotIndex]->setStyleSheet("background-color:rgb(143,89,2)");
    }
        break;
    case ROBOT_CLEARING:
    {
        robotLabelArr[robotIndex]->setText("CLEARING");
        robotLabelArr[robotIndex]->setStyleSheet("background:blue");
    }
        break;
    case ULTRASONIC_ROBOT1_UNRECOGNIZED:
    {
        robotLabelArr[robotIndex]->setText("ROBOT1_UNRECOGNIZED");
        robotLabelArr[robotIndex]->setStyleSheet("background-color:rgb(143,89,2)");
    }
        break;

    case ULTRASONIC_ROBOT2_UNRECOGNIZED:
    {
        robotLabelArr[robotIndex]->setText("ROBOT2_UNRECOGNIZED");
        robotLabelArr[robotIndex]->setStyleSheet("background:red");
    }
        break;
    case ULTRA_ROBOT_ACK_INSERT_CATCH:
    {
        robotLabelArr[robotIndex]->setText("ULTRA_ACK_INSERT_CATCH");
        robotLabelArr[robotIndex]->setStyleSheet("background:green");
    }
        break;
    case ULTRA_ROBOT_ACK_INSERT:
    {
        robotLabelArr[robotIndex]->setText("ULTRA_ACK_INSERT");
        robotLabelArr[robotIndex]->setStyleSheet("background:green");
    }
        break;
    case ULTRA_ROBOT_WORKING:
    {
        robotLabelArr[robotIndex]->setText("ULTRA_ROBOT_WORKING");
        robotLabelArr[robotIndex]->setStyleSheet("background:green");
    }
        break;
    case ULTRA_ROBOT_ACK_CATCH:
    {
        robotLabelArr[robotIndex]->setText("ULTRA_ACK_CATCH");
        robotLabelArr[robotIndex]->setStyleSheet("background:green");
    }
        break;
    case ULTRA_ROBOT_ACK_CATCH_ERROR:
    {
        robotLabelArr[robotIndex]->setText("ACK_CATCH_ERROR");
        robotLabelArr[robotIndex]->setStyleSheet("background:red");
    }
        break;
    case ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR:
    {
        robotLabelArr[robotIndex]->setText("ACK_CATCH_INSERT_ERROR");
        robotLabelArr[robotIndex]->setStyleSheet("background:red");
    }
        break;
    case ULTRA_ROBOT_ACK_INSERT_ERROR:
    {
        robotLabelArr[robotIndex]->setText("ACK_INSERT_ERROR");
        robotLabelArr[robotIndex]->setStyleSheet("background:red");
    }
        break;
    default:
        break;
    }
}

void SchedulerGUI::updateRobotUI()
{
    for(uint i=0;i<robotNum;++i){
        updateSingleRobot(i,robotWorkStateArray[i]);
    }
}

void SchedulerGUI::setRobotLabelPosition()
{
    uint robotNumber=0;
    uint startDrawX=conveyorLabel->x();
    uint startDrawY=conveyorLabel->y()+conveyorLabel->height()/2;

    for(uint i=0;i<moduleNum;++i){
        uint drawX=startDrawX+i*(modelRadius*2);
        uint drawY=startDrawY;

        bool isRobotModulePosition=false;
        for(uint j=0;j<robotNum;++j)
        {
            isRobotModulePosition=isRobotModulePosition||(i==robotModuleNumber[j]);
        }
        if(isRobotModulePosition)
        {
            switch (robotNumber)
            {
            case 0:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY+labelGap-modelRadius,robotLabelWidth,robotLabelHeight);
            }
                break;
            case 1:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY-labelGap-modelRadius*2,robotLabelWidth,robotLabelHeight);
            }
                break;
            case 2:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY+labelGap-modelRadius,robotLabelWidth,robotLabelHeight);
            }
                break;
            case 3:
            {
                robotLabelArr[robotNumber]->setGeometry(drawX-robotLabelWidth/2+modelRadius/2,drawY-labelGap-modelRadius*2,robotLabelWidth,robotLabelHeight);
            }
                break;
            default:
                break;
            }
            ++robotNumber;
        }
    }
}

void SchedulerGUI::iniModuleLabel()
{
    uint rowModuleNum=2;
    lineModuleNum=(moduleNum-rowModuleNum*2)/2;

    uint startX=conveyorLabel->x();
    uint startY=conveyorLabel->y();

    for(uint i=0;i<moduleNum;++i){
        bool isRobotPosition=false;
        for(uint j=0;j<robotNum;++j){
            isRobotPosition=isRobotPosition||(i==robotModuleNumber[j]);
        }
        uint x;
        uint y;
        if(i<lineModuleNum)
        {
            x=startX+((i-1)*modelRadius*2);
            y=startY;
            moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
        }
        else if(i>=lineModuleNum&&i<(lineModuleNum+rowModuleNum)){
            x=startX+(lineModuleNum-2)*modelRadius*2;
            y=startY+(i-lineModuleNum+1)*modelRadius*2;
            moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
        }
        else if(i>=(lineModuleNum+rowModuleNum)&&i<(lineModuleNum*2+rowModuleNum)){

            x=startX+((moduleNum-i-4)*modelRadius*2);
            y=startY+120+modelRadius;
            moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
        }
        else
        {
            x=startX-modelRadius*2;
            y=startY+(moduleNum-i)*modelRadius*2;
            moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
        }

        //        moduleLabelArray[i]->setText(QString::number(i+1,10));
        //        moduleLabelArray[i]->setAlignment(Qt::AlignCenter);
        QFont test;
        test.setBold(true);
        moduleLabelArray[i]->setFont(test);
        moduleLabelArray[i]->setGeometry(x,y,modelRadius+15,modelRadius);
        moduleLabelArray[i]->setFrameShape(QFrame::Box);
        moduleLabelArray[i]->setFrameShadow(QFrame::Sunken);
        if(isRobotPosition)
        {
            moduleLabelArray[i]->setLineWidth(8);
            if(i==robotModuleNumber[ultrasonicIndex])
                moduleLabelArray[i+1]->setLineWidth(8);
        }
    }

    moduleLabelArray[detectSensorPosition]->setLineWidth(8);
}

void SchedulerGUI::connectSlotAndSignal()
{
    connect(ioUI,SIGNAL(signalControlConveyor(uint,uint)),this,SLOT(slotSetConveyorStatus(uint,uint)));
    connect(ioUI,SIGNAL(signalControlPlate(uint,uint)),this,SLOT(slotControlPlate(uint,uint)));
    connect(ioUI,SIGNAL(signalPressAllPlate()),this,SLOT(slotPressAllPlate()));
    connect(ioUI,SIGNAL(signalReleaseAllPlate()),this,SLOT(slotReleaseAllPlate()));


    connect(&checkCatchableTimer,SIGNAL(timeout()),this,SLOT(updateModuleArrayState()));
    connect(this,SIGNAL(signalAddAdapter(uint)),this,SLOT(slotAddAdapter(uint)));
    connect(&checkResetSignalTimer,SIGNAL(timeout()),this,SLOT(checkResetButtonStatus()));
    connect(&alarmTimer,SIGNAL(timeout()),this,SLOT(slotAlarmTimerTimeout()));
    //////////////connect robot timer msg////////////////////////////
    for(uint i=0;i<robotNum;++i){
        connect(robotMsgTimer[i],SIGNAL(timerOutTime(uint)),this,SLOT(robotReplyMsgTimeOut(uint)));
    }
}

void SchedulerGUI::closeServer()
{
    for(uint i=0;i<robotNum;++i)
    {
        slotRobotOffLine(i);
    }
    server->close();
}

void SchedulerGUI::sendOrderToUltrasonicRobot(QString msg)
{
    if(isUltrasonicOnline)
    {
        addRunMessage("send order to ultrasonic robot:"+msg,Qt::blue);
        slotSendMsg(ultrasonicIndex,msg);
    }
}

bool SchedulerGUI::serverIni()
{
    server=new QTcpServer(this);
    if(server->listen(QHostAddress::Any, 8888))
    {
        connect(server,SIGNAL(newConnection()),this,SLOT(slotNewConnection()));
        addRunMessage("server initialize success",Qt::green);
    }
    else
    {
        QMessageBox::warning(NULL,"Warning","server initialize fail,please check the network then restart this program!");
        exit(0);
    }
}

void SchedulerGUI::ultrasonicServerIni()
{
    ultraThread=new ServerThread();
//    connect(ultraThread,SIGNAL(ultraReceivedMsg(QString)),this,SLOT(slotReceivedMsgFromUltra(QString)));
//    connect(ultraThread,SIGNAL(ultraConnected()),this,SLOT(slotUltrasonicRobotOnline()));
//    connect(ultraThread,SIGNAL(ultraDisconnected()),this,SLOT(slotUltrasonicRobotOffLine()));

    connect(&(ultraThread->ultraServer),SIGNAL(ultraReceivedMsg(QString)),this,SLOT(slotReceivedMsgFromUltra(QString)));
    connect(&(ultraThread->ultraServer),SIGNAL(ultraConnected()),this,SLOT(slotUltrasonicRobotOnline()));
    connect(&(ultraThread->ultraServer),SIGNAL(ultraDisconnected()),this,SLOT(slotUltrasonicRobotOffLine()));
    connect(&(ultraThread->ultraServer),SIGNAL(ultraServerIniError()),this,SLOT(slotUltraServerIniError()));

    ultraThread->start();
}

void SchedulerGUI::sendCatchOrder(uint robotNumber)
{
    if(isOnlyCatch)
    {
//        qDebug()<<"order:"<<dropOrder;
        if(robotSocketArray[robotNumber]->sendMsg(dropOrder))
        {
            slotRobotCatching(robotNumber);
        }
        else
        {
            addRunMessage("robot "+QString::number(robotNumber,10)+" send catch order error!",Qt::red);
            slotRobotOffLine(robotNumber);
        }
        return;
    }
    else{
        if(robotSocketArray[robotNumber]->sendMsg(catchOrder))
        {
            slotRobotCatching(robotNumber);
        }
        else
        {
            addRunMessage("robot "+QString::number(robotNumber,10)+" send catch order error!",Qt::red);
            slotRobotOffLine(robotNumber);
        }
    }
}

void SchedulerGUI::sendClearOrder(uint robotNumber)
{
    if(!robotSocketArray[robotNumber]->sendMsg(clearOrder))
    {
        addRunMessage("robot "+QString::number(robotNumber,10)+" send clear order error!",Qt::red);
        slotRobotOffLine(robotNumber);
    }
}

void SchedulerGUI::emergencyStop()
{
    isEmergencyStop=true;
    stopConveyor();
    moduleStateArray[moduleNum-1]=MODULE_ERROR;
    updateSingleModule(moduleNum-1,MODULE_ERROR);
    slotSetAlarmStatus(START_ALARM,ALARM_CONTINUOUS);
    addRunMessage("Emergency Stop!",Qt::red);
}

void SchedulerGUI::stopConveyor()
{
    isConveyorRunning=false;
    slotSetConveyorStatus(CONVEYOR_STOP,CONVEYOR_STOP);
}

void SchedulerGUI::activateConveyor()
{
    isConveyorRunning=true;
    slotSetConveyorStatus(CONVEYOR_ACTIVATE,CONVEYOR_FORWARD);
}

void SchedulerGUI::removeRobotSocket(uint robotNumber)
{
    if(robotSocketArray.find(robotNumber)!=robotSocketArray.end())
    {
        delete robotSocketArray[robotNumber];
        robotSocketArray.erase(robotSocketArray.find(robotNumber));
        if(robotNumber==ultrasonicIndex)
        {
            isUltrasonicOnline=false;
            addRunMessage("ultrasonic offline!",Qt::red);
        }
        else
        {
            --onlineTestRobotNum;
            if(lastOnlineTestRobotNumber==robotNumber)
            {
                if(onlineTestRobotNum==0)
                {
                    lastOnlineTestRobotNumber=-1;
                }
                else
                {
                    lastOnlineTestRobotNumber=-1;
                    auto iter=robotSocketArray.begin();
                    while(iter!=robotSocketArray.end())
                    {
                        if(((int)iter->first)>lastOnlineTestRobotNumber
                                &&((int)iter->first)!=ultrasonicIndex)
                        {
                            lastOnlineTestRobotNumber=iter->first;
                            //qDebug()<<"last online test robot:"<<lastOnlineTestRobotNumber;
                        }
                        ++iter;
                    }
                }
            }

            addRunMessage("last online test robot:"+QString::number(lastOnlineTestRobotNumber,10),Qt::blue);
        }
    }
}

void SchedulerGUI::addRobotSocket(uint robotNumber, Socket *_socket)
{
    removeRobotSocket(robotNumber);
    robotSocketArray.insert(std::pair<uint,Socket*>(robotNumber,_socket));
    if(robotNumber==ultrasonicIndex)
    {
        isUltrasonicOnline=true;
        addRunMessage("ultrasonic online!",Qt::blue);
    }
    else
    {
        ++onlineTestRobotNum;
        lastOnlineTestRobotNumber=((int)robotNumber)>lastOnlineTestRobotNumber?robotNumber:lastOnlineTestRobotNumber;
        addRunMessage("last online test robot:"+QString::number(lastOnlineTestRobotNumber,10),Qt::blue);
    }
}

void SchedulerGUI::updateModuleArrayState()
{
    if(!isConveyorRunning)
    {
        checkCatchable();
    }

    bool currR1ModuleSensorStatus=gpio_pi.getIOStatus(robotModuleSensorIO[0]);
    if(
            (!isDetectFallingEdge)&&
            ((currR1ModuleSensorStatus&&prevR1ModuleSensorStatus&&isConveyorRunning)
             ||isEmergencyStop
             ))
    {
        if(!isEmergencyStop)
        {
            ++detectAdapterTime;
        }

        if(detectAdapterTime<maxDetectAdapterTime&&!isHaveAdapter)
        {
            bool currR1AdapterSensorStatus=gpio_pi.getIOStatus(robotAdapterSensorIO[0]);
            isHaveAdapter=currR1AdapterSensorStatus&&prevR1AdapterSensorStatus;
            if(isHaveAdapter)
            {
                if(isProductiveTwoModel)
                {
                    if(!isStartCount){
                        isStartCount=true;
                        isValidModule=true;
                    }
                }
            }
            prevR1AdapterSensorStatus=currR1AdapterSensorStatus;
        }
        else if(detectAdapterTime>=maxDetectAdapterTime)
        {
            if(isProductiveTwoModel)
            {
                if(isValidModule&&isStartCount&&!isHaveAdapter)
                {
                    emit signalAddAdapter(MODULE_EMPTY);
                }
                else if(isValidModule&&isStartCount&&isHaveAdapter)
                {
                    emit signalAddAdapter(MODULE_LOADED);
                }
            }
            else
            {

                if(isHaveAdapter)
                {
                    emit signalAddAdapter(MODULE_LOADED);
                }
                else
                {
                    emit signalAddAdapter(MODULE_EMPTY);
                }
            }
            isDetectFallingEdge=true;
        }
    }

    ///////begin to detect is the module leave after detected a module come//////
    if((isDetectFallingEdge&&isConveyorRunning)||isEmergencyStop)
    {
        if(!(prevR1ModuleSensorStatus||currR1ModuleSensorStatus))
        {
            if(isProductiveTwoModel)
            {
                isValidModule=!isValidModule;
            }
            isDetectFallingEdge=false;
            isHaveAdapter=false;
            prevR1AdapterSensorStatus=false;
            detectAdapterTime=0;
        }
    }
    prevR1ModuleSensorStatus=currR1ModuleSensorStatus;
    checkIsActivateConveyor();
}

void SchedulerGUI::checkIsActivateConveyor()
{
    /////////////////////confirm is stop conveyor////////////////////////
    if(onlineTestRobotNum==0)
    {
//        qDebug()<<"onlineTestRobotNum==0";
        if(isConveyorRunning)
            stopConveyor();
        return;
    }

    if(isUltraValid&&!isUltrasonicOnline)
    {
//           qDebug()<<"isUltraValid&&!isUltrasonicOnline";
        if(isConveyorRunning)
            stopConveyor();
        return;
    }

    if(robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT_CATCH||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_WORKING
            )
    {
//            qDebug()<<"robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH||";
        if(isConveyorRunning&&
                detectAdapterTime<maxDetectAdapterTime&&detectAdapterTime!=0)
            stopConveyor();
        return;
    }

    if(robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH_ERROR||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT_ERROR||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR
            )
    {
//         qDebug()<<"robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH_ERROR";
        if(isConveyorRunning)
            stopConveyor();
        return;
    }

    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ULTRA1_CAUGHT)
    {
        if(isConveyorRunning)
            stopConveyor();
        return;
    }


    if(isStopLastRobot){
        if(moduleStateArray[robotModuleNumber[lastOnlineTestRobotNumber]]==MODULE_ULTRASONIC||
                (moduleStateArray[robotModuleNumber[lastOnlineTestRobotNumber]]==MODULE_UNRECOGNIZE_ONCETIME
                 &&robotWorkStateArray[lastOnlineTestRobotNumber]!=ROBOT_RECOGNIZE_ERROR))
        {
            if(isConveyorRunning&&
                    detectAdapterTime!=0&&
                    detectAdapterTime<=maxDetectAdapterTime)
            {
//                addRunMessage("last robot module number:"+
//                              QString::number(robotModuleNumber[lastOnlineTestRobotNumber],10),Qt::blue);
//                addRunMessage("last robot has a module!",Qt::red);
                stopConveyor();
            }
            return;
        }
    }

    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]]==MODULE_LOADED)
    {
        if(isConveyorRunning)
            stopConveyor();
        return;
    }

    if(isEmergencyStop)
    {
        if(isConveyorRunning)
            stopConveyor();
        return;
    }

    for(uint i=0;i<robotNum;++i)
    {
        if(i==ultrasonicIndex)
            continue;
        if(robotWorkStateArray[i]==ROBOT_ACK_CATCHED_ERROR||robotWorkStateArray[i]==ROBOT_ACK_CATCHED)
        {
            if(isConveyorRunning)
                stopConveyor();
            return;
        }
    }
    activateConveyor();
}

void SchedulerGUI::clearError()
{
    clearAckCatchedError();
    //    clearLastModuleHaveAdapterError();
}

void SchedulerGUI::clearAckCatchedError()
{
    if(robotWorkStateArray[ultrasonicIndex]==ROBOT_ACK_CATCHED_ERROR)
    {
        QString str=QInputDialog::getText(NULL,"Warning","Please input password(qwer1234) to clear the ultrasonic error!",QLineEdit::Password);
        if(str=="qwer1234"){
            // robotWorkStateArray[ultrasonicIndex]=ROBOT_OFFLINE;
            slotRobotStateChanged(ultrasonicIndex,ROBOT_OFFLINE);
        }
        else
            return;
    }

    /////////////////confirm the robot is in a safe status then you can activate the conveyor//////
    for(uint i=0;i<robotNum;++i)
    {
        if(robotWorkStateArray[i]==ROBOT_ACK_CATCHED_ERROR)
        {
            slotRobotStateChanged(i,ROBOT_OFFLINE);
            // robotWorkStateArray[i]=ROBOT_OFFLINE;
        }
    }

    slotSetAlarmStatus(STOP_ALARM,STOP_ALARM);
    isEmergencyStop=false;
    addRunMessage("Clear Ack Catched Error!",Qt::blue);
    // updateRobotUI();
}

void SchedulerGUI::clearLastModuleHaveAdapterError()
{
    bool isEmergencyAdapterExists=gpio_pi.getIOStatus(robotAdapterSensorIO[3]);
    if(isEmergencyAdapterExists==true)
    {
        addRunMessage("Please clear the adapter first!",Qt::red);
        return;
    }

    if(moduleStateArray[moduleNum-2]==MODULE_LOADED||moduleStateArray[moduleNum-2]==MODULE_UNRECOGNIZE_ONCETIME)
        moduleStateArray[moduleNum-2]=MODULE_EMPTY;

    slotSetAlarmStatus(STOP_ALARM,STOP_ALARM);
    isEmergencyStop=false;
    addRunMessage("Clear Last Module Have Adapter Error!",Qt::blue);
}

void SchedulerGUI::checkResetButtonStatus()
{
    bool _isPush=gpio_pi.getIOStatus(resetAlarmButtonIO);
    isPushResetButton=isPushResetButton&&_isPush;
    if(isPushResetButton)
    {
        clearLastModuleHaveAdapterError();
        addRunMessage("Detected push the reset button!",Qt::blue);
        isPushResetButton=false;
        return;
    }
    isPushResetButton=_isPush;
}

void SchedulerGUI::slotRobotWaiting(uint robotNumber)
{
    if(robotWorkStateArray[robotNumber]==ROBOT_ACK_CATCHED||
            robotWorkStateArray[robotNumber]==ULTRA_ROBOT_ACK_INSERT_CATCH||
            robotWorkStateArray[robotNumber]==ULTRA_ROBOT_ACK_INSERT||
            robotWorkStateArray[robotNumber]==ULTRA_ROBOT_ACK_CATCH
            )
    {
//        addRunMessage(QString::number(robotWorkStateArray[robotNumber],10)+" receive wait command in a error state!",Qt::red);
        return;
    }
    slotRobotStateChanged(robotNumber,ROBOT_WAITING);
}

void SchedulerGUI::slotRobotCatching(uint robotNumber)
{
    if(moduleStateArray[robotModuleNumber[robotNumber]]!=MODULE_UNRECOGNIZE_ONCETIME)
        slotModuleStateChanged(robotModuleNumber[robotNumber],MODULE_CATCHING);
    slotRobotStateChanged(robotNumber,ROBOT_ACK_CATCHED);
    ////////start the ack catched timer//////////////////
    robotMsgTimer[robotNumber]->setTimerData(robotNumber,ROBOT_ACK_CATCHED);
    robotMsgTimer[robotNumber]->start(checkCatchedMsgInterval);
}

void SchedulerGUI::slotRobotCatched(uint robotNumber)
{
    if(robotWorkStateArray[robotNumber]!=ROBOT_ACK_CATCHED&&
            robotWorkStateArray[robotNumber]!=ULTRA_ROBOT_ACK_CATCH&&
            robotWorkStateArray[robotNumber]!=ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        addRunMessage(QString::number(robotWorkStateArray[robotNumber],10)+" robot receive caught command in a error state",Qt::red);
        return;
    }

    if(robotNumber==ultrasonicIndex)
        slotUltraRobot1Caught();
    else
    {
        slotRobotStateChanged(robotNumber,ROBOT_CATCHED);
        slotModuleStateChanged(robotModuleNumber[robotNumber],MODULE_EMPTY);
        //////////stop the ack catched timer/////////////////
        robotMsgTimer[robotNumber]->setTimerData(robotNumber,ROBOT_CATCHED);
        robotMsgTimer[robotNumber]->stop();
    }
    statisticAdapter(robotNumber,ROBOT_CATCHED);
}

void SchedulerGUI::slotAddAdapter(uint moduleState)
{
    for(uint i=0;i<robotNum;++i)
    {
        if(robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
            robotWorkStateArray[i]=ROBOT_WAITING;
    }
    if(robotWorkStateArray[ultrasonicIndex]==ULTRASONIC_ROBOT1_UNRECOGNIZED)
        robotWorkStateArray[ultrasonicIndex]=ROBOT_WAITING;

    uint sensorModulePrevStatu=moduleStateArray[detectSensorPosition-1];
    uint lastModuleState=moduleStateArray[moduleNum-1];
    for(uint i=moduleNum-1;i>0;--i)
    {
        moduleStateArray[i]=moduleStateArray[i-1];
    }
    moduleStateArray[0]=lastModuleState;

    if(isUltraValid)
    {
        if(sensorModulePrevStatu==MODULE_ULTRASONIC||
                sensorModulePrevStatu==MODULE_UNRECOGNIZE_ONCETIME||
                sensorModulePrevStatu==MODULE_UNRECOGNIZE_TWICETIME
                )
        {
            if(moduleState==MODULE_LOADED)
//                moduleStateArray[detectSensorPosition]=MODULE_ULTRASONIC;
                moduleStateArray[detectSensorPosition]=MODULE_LOADED;
            else if(moduleState==MODULE_EMPTY)
                moduleStateArray[detectSensorPosition]=MODULE_EMPTY;
        }
        else
            moduleStateArray[detectSensorPosition]=moduleState;
    }
    else
    {
        if(moduleState==MODULE_LOADED)
            moduleStateArray[detectSensorPosition]=MODULE_ULTRASONIC;
        else if(moduleState==MODULE_EMPTY)
            moduleStateArray[detectSensorPosition]=MODULE_EMPTY;
    }

    checkIsActivateConveyor();
    checkCatchable();
    updateUI();
}

void SchedulerGUI::slotRobotOnLine(uint robotNumber)
{
    if(robotNumber>robotNum-1)
    {
        QMessageBox::critical(NULL,"WARNING","input robotnumber error!");
        exit(0);
    }

    if(robotNumber==ultrasonicIndex)
    {
        slotUltrasonicRobotOnline();
    }
    slotRobotStateChanged(robotNumber,ROBOT_CONNECTED);
}

void SchedulerGUI::slotUltrasonicRobotOnline()
{
    isUltrasonicOnline=true;
    slotRobotStateChanged(ultrasonicIndex,ROBOT_CONNECTED);
    //    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ULTRA2_UNRECOGNIZED)
    //    {
    //        moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]=MODULE_EMPTY;
    //        moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_EMPTY;
    //    }

//    if(robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR||
//            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT_ERROR)
//    {
//        if(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ERROR)
//            moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]=MODULE_ULTRASONIC;
//    }
}

void SchedulerGUI::slotUltrasonicRobotOffLine()
{
    isUltrasonicOnline=false;
    if(robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT_CATCH||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT||
            robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH
            )
    {
        slotUltraRobotAckError(robotWorkStateArray[ultrasonicIndex]);
    }
    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ULTRA1_CAUGHT)
    {
        slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+1,MODULE_ERROR);
    }
    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]]==MODULE_ULTRA1_CAUGHT)
    {
        slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_ERROR);
    }
    
    slotRobotStateChanged(ultrasonicIndex,ROBOT_OFFLINE);
    removeRobotSocket(ultrasonicIndex);
}

void SchedulerGUI::slotRobotOffLine(uint robotNumber)
{
    ////offline when the robot is catching module////
    if(robotWorkStateArray[robotNumber]==ROBOT_ACK_CATCHED&&robotNumber!=ultrasonicIndex)
    {
        slotTestRobotAckCatchedError(robotNumber);
        return;
    }
    if(robotNumber==ultrasonicIndex)
    {
        slotUltrasonicRobotOffLine();
        return;
    }

    slotRobotStateChanged(robotNumber,ROBOT_OFFLINE);
    removeRobotSocket(robotNumber);
    if(robotNumber>robotNum-1)
    {
        QMessageBox::critical(NULL,"WARNING","input robotnumber error!");
        exit(0);
    }
}

void SchedulerGUI::slotTestRobotAckCatchedError(uint robotNumber)
{
    if(robotNumber>robotNum-1)
    {
        QMessageBox::critical(NULL,"WARNING","input robotnumber error!");
        exit(0);
    }
    slotModuleStateChanged(robotModuleNumber[robotNumber],MODULE_ERROR);
    slotRobotStateChanged(robotNumber,ROBOT_ACK_CATCHED_ERROR);
    robotMsgTimer[robotNumber]->stop();
    stopConveyor();
    slotSetAlarmStatus(START_ALARM,ALARM_FLASH);
    removeRobotSocket(robotNumber);
    if(robotNumber!=ultrasonicIndex){
        addRunMessage(QString::number(robotNumber,10)+" ack catched error!",Qt::red);
    }
    else if(robotNumber==ultrasonicIndex)
    {
        addRunMessage("ultrasonic ack catched error!",Qt::red);
    }
}

void SchedulerGUI::slotTestRobotRecognizeError(uint robotNumber)
{
    if(robotWorkStateArray[robotNumber]!=ROBOT_ACK_CATCHED)
    {
        addRunMessage(QString::number(robotWorkStateArray[robotNumber],10)+" robot receive unrecognized command in a error state!",Qt::red);
        return;
    }

    if(moduleStateArray[robotModuleNumber[robotNumber]]==MODULE_UNRECOGNIZE_ONCETIME
            ||moduleStateArray[robotModuleNumber[robotNumber]]==MODULE_CATCHING)
    {
        testRobotUnrecognized(robotNumber);
    }
    statisticAdapter(robotNumber,ROBOT_RECOGNIZE_ERROR);
    robotMsgTimer[robotNumber]->stop();
}

void SchedulerGUI::slotTestRobotClearing(uint robotNumber)
{
    if(robotWorkStateArray[robotNumber]==ROBOT_WAITING)
    {
        slotRobotStateChanged(robotNumber,ROBOT_CLEARING);
    }
    else
        addRunMessage(QString::number(robotWorkStateArray[robotNumber],10)+"  Robot receive a clear order when it isn't in the waitting status!",Qt::red);
}

void SchedulerGUI::slotControlPlate(uint robotNumber, uint status)
{
    //    gpio_pi.setIOStatus(robotPlateIO[robotNumber],status);
}

void SchedulerGUI::slotSetConveyorStatus(uint status,uint direction)
{
    if(status==CONVEYOR_ACTIVATE)
        slotReleaseAllPlate();
    gpio_pi.setIOStatus(conveyorStatusIO,status);
}

void SchedulerGUI::slotReleaseAllPlate()
{
    for(uint i=0;i<robotNum;++i)
    {
        gpio_pi.setIOStatus(robotPlateIO[i],PLATE_RELEASE);
    }
}

void SchedulerGUI::slotPressAllPlate()
{
    for(uint i=0;i<robotNum;++i)
    {
        gpio_pi.setIOStatus(robotPlateIO[i],PLATE_PRESS);
    }
}

void SchedulerGUI::slotSetAlarmStatus(bool status,uint alarmFlag)
{
    return;
    //    alarmStatus=status;
    //    if(status==START_ALARM)
    //    {
    //        if(alarmFlag==ALARM_CONTINUOUS){
    //            if(!checkResetSignalTimer.isActive())
    //            {
    //                checkResetSignalTimer.setInterval(checkResetButtonInterval);
    //                checkResetSignalTimer.start();
    //            }
    //        }
    //        gpio_pi.setIOStatus(alarmStatusIO,START_ALARM);
    //        addRunMessage("start alarm",Qt::red);
    //        if(alarmFlag==ALARM_FLASH)
    //        {
    //            if(!alarmTimer.isActive())
    //            {
    //                alarmTimer.setInterval(flashAlarmInterval);
    //                alarmTimer.start();
    //            }
    //        }
    //    }
    //    else
    //    {
    //        addRunMessage("stop alarm",Qt::blue);
    //        checkResetSignalTimer.stop();
    //        alarmTimer.stop();
    //        gpio_pi.setIOStatus(alarmStatusIO,STOP_ALARM);
    //    }
}

void SchedulerGUI::slotAlarmTimerTimeout()
{
    if(alarmStatus==START_ALARM)
    {
        gpio_pi.setIOStatus(alarmStatusIO,STOP_ALARM);
        alarmStatus=STOP_ALARM;
    }
    else
    {
        gpio_pi.setIOStatus(alarmStatusIO,START_ALARM);
        alarmStatus=START_ALARM;
    }
}

void SchedulerGUI::slotUltraRobotWorking(uint workState)
{
    robotMsgTimer[ultrasonicIndex]->start(checkCatchedMsgInterval);
    slotRobotStateChanged(ultrasonicIndex,workState);
    ////////start the ack catched timer//////////////////
}

void SchedulerGUI::slotUltraRobot1Caught()
{
    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]]==MODULE_CATCHING)
        slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_ULTRA1_CAUGHT);
    else{
        return;
    }
    if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_CATCH)
    {
        robotMsgTimer[ultrasonicIndex]->stop();
        slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_WORKING);
    }
    else if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_INSERT);
        slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_ACK_INSERT);
    }
}

void SchedulerGUI::slotUltraRobot2Inserted()
{
    if(robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT&&
            robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        //        addRunMessage(QString::number(robotWorkStateArray[ultrasonicIndex],10)+
        //                      " ultra robot receive command inserted in a error state!",Qt::red);
        return;
    }

    if(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ULTRA1_CAUGHT)
    {
        slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+1,MODULE_ULTRASONIC);
        if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_INSERT)
        {
            robotMsgTimer[ultrasonicIndex]->stop();
            slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_WORKING);
        }
        else if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_INSERT_CATCH)
        {
            slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH);
            robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH);
        }
    }
    else
    {
        addRunMessage("ultrasonic robot can't insert adapter to a module that not in the MODULE_ULTRA1_CAUGHT state!"+
                      QString::number(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1],10),Qt::red);
        return;
    }
}

void SchedulerGUI::slotUltraRobot1Unrecognized()
{
    if(robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_CATCH&&
            robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        addRunMessage(QString::number(robotWorkStateArray[ultrasonicIndex],10)+" ultra robot1 received command unrecognized in a error state!",Qt::red);
        return;
    }
    if(robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_CATCH)
        slotRobotStateChanged(ultrasonicIndex,ULTRASONIC_ROBOT1_UNRECOGNIZED);
    else if(robotWorkStateArray[ultrasonicIndex]==ULTRA_ROBOT_ACK_INSERT_CATCH){
        slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_ACK_INSERT);
    }
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_ULTRA1_UNRECOGNIZED);

    addRunMessage("ultrasonic robot1 unrecognized!",Qt::red);
    statisticAdapter(ultrasonicIndex,ROBOT_RECOGNIZE_ERROR);
    if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_CATCH)
        robotMsgTimer[ultrasonicIndex]->stop();
    else if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_INSERT);
    }
}

void SchedulerGUI::slotUltraRobot2Unrecognized()
{
    if(robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT&&
            robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT_CATCH
            )
    {
        addRunMessage(QString::number(robotWorkStateArray[ultrasonicIndex],10)+" receive a error msg robot2 unrecognized in a error state!",Qt::red);
        return;
    }
    if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_INSERT)
        robotMsgTimer[ultrasonicIndex]->stop();
    else if(robotMsgTimer[ultrasonicIndex]->getTimerDescriptor()==ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH);
    }

    statisticAdapter(ultrasonicIndex,ROBOT_RECOGNIZE_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+1,MODULE_ULTRA2_UNRECOGNIZED);
    addRunMessage("ultrasonic robot2 unrecognized!",Qt::red);
    slotRobotOffLine(ultrasonicIndex);
}

void SchedulerGUI::slotUltraRobotAckError(uint ack_state)
{
   switch (ack_state){
   case ULTRA_ROBOT_ACK_INSERT_CATCH:{
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_ERROR;
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]=MODULE_ERROR;
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]+2]=MODULE_ERROR;
//        robotWorkStateArray[ultrasonicIndex]=ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR;
       addRunMessage("ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR!",Qt::red);
   }
       break;
   case ULTRA_ROBOT_ACK_INSERT:{
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]=MODULE_ERROR;
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]+2]=MODULE_ERROR;
//        if(moduleStateArray[robotModuleNumber[ultrasonicIndex]]==MODULE_ULTRA1_CAUGHT)
//            moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_ERROR;
//        robotWorkStateArray[ultrasonicIndex]=ULTRA_ROBOT_ACK_INSERT_ERROR;
       addRunMessage("ULTRA_ROBOT_ACK_INSERT_ERROR!",Qt::red);
   }
       break;
   case ULTRA_ROBOT_ACK_CATCH:{
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_ERROR;
//        moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]=MODULE_ERROR;
//        if(moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ULTRA1_CAUGHT){
//            moduleStateArray[robotModuleNumber[ultrasonicIndex]+2]=MODULE_ERROR;
//        }
//        robotWorkStateArray[ultrasonicIndex]=ULTRA_ROBOT_ACK_CATCH_ERROR;
       addRunMessage("ULTRA_ROBOT_ACK_CATCH_ERROR!",Qt::red);
   }
       break;
//    default:
//        break;
   }

    // robotWorkStateArray[ultrasonicIndex]=ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR;
    slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+1,MODULE_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+2,MODULE_ERROR);

    // moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_ERROR;
    // moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]=MODULE_ERROR;
    // moduleStateArray[robotModuleNumber[ultrasonicIndex]+2]=MODULE_ERROR;

    robotMsgTimer[ultrasonicIndex]->stop();
    stopConveyor();
    slotSetAlarmStatus(START_ALARM,ALARM_FLASH);
    removeRobotSocket(ultrasonicIndex);
    // updateUI();
}

void SchedulerGUI::slotUltraRobot2InsertError()
{
    if(robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT&&robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        addRunMessage("ultrasonic robot receive a error command ur2_insert_error!",Qt::red);
        return;
    }
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+1,MODULE_ERROR);
    slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR);

    addRunMessage("ultra robot2 insert error!",Qt::red);

    robotMsgTimer[ultrasonicIndex]->stop();
    stopConveyor();
    slotSetAlarmStatus(START_ALARM,ALARM_FLASH);
    removeRobotSocket(ultrasonicIndex);
}

void SchedulerGUI::slotUltraRobot1CatchError(){
    if(robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_CATCH&&robotWorkStateArray[ultrasonicIndex]!=ULTRA_ROBOT_ACK_INSERT_CATCH)
    {
        addRunMessage("ultrasonic robot receive a error command ur1_catch_error!",Qt::red);
        return;
    }

    slotRobotStateChanged(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_ERROR);
    slotModuleStateChanged(robotModuleNumber[ultrasonicIndex]+1,MODULE_ERROR);
    addRunMessage("ultra robot1 catch error!",Qt::red);

    robotMsgTimer[ultrasonicIndex]->stop();
    stopConveyor();
    slotSetAlarmStatus(START_ALARM,ALARM_FLASH);
    removeRobotSocket(ultrasonicIndex);
}

void SchedulerGUI::slotReceivedMsgFromUltra(QString msg)
{
//    qDebug()<<"received msg from ultra:"<<msg;
    processMsgFromRobot(ultrasonicIndex,msg);
}

void SchedulerGUI::slotUltraServerIniError()
{
    addRunMessage("ULTRA SERVER INI ERROR!",Qt::red);
}


void SchedulerGUI::slotRobotStateChanged(uint robotIndex,uint robotState){
    robotWorkStateArray[robotIndex]=robotState;
    updateSingleRobot(robotIndex,robotState);
}

void SchedulerGUI::slotModuleStateChanged(uint moduleIndex,uint moduleState){
    moduleStateArray[moduleIndex]=moduleState;
    updateSingleModule(moduleIndex,moduleState);
}

void SchedulerGUI::slotNewConnection()
{
    QTcpSocket* clientSocket=server->nextPendingConnection();
    Socket* robotSocket=new Socket;
    robotSocket->setSocket(clientSocket);

    QString socketIP="";
    socketIP=clientSocket->peerAddress().toString();
    socketIP=socketIP.split(":").last();
    int newRobotNumber=-1;
    for(uint i=0;i<robotNum;++i)
    {
        if(socketIP==robotIP[i])
        {
            newRobotNumber=i;
            robotSocket->setSocketFlag(newRobotNumber);
            addRobotSocket(newRobotNumber,robotSocket);
            slotRobotOnLine(newRobotNumber);
            connect(robotSocket,SIGNAL(readyToRead(uint)),this,SLOT(slotReceiveMsg(uint)));
            connect(robotSocket,SIGNAL(disconnectServer(uint)),this,SLOT(slotDisconnect(uint)));
            break;
        }
    }

    ///////////if there is a old robot connect ,remove it//////////
    if(newRobotNumber==-1)
    {
        addRunMessage("A new connection from a uncertain ip:"+socketIP,Qt::red);
        clientSocket->close();
    }
}

void SchedulerGUI::slotReceiveMsg(uint robotNumber)
{
    QString msg;
    if(!robotSocketArray[robotNumber]->readMsg(msg))
    {
        slotRobotOffLine(robotNumber);
        addRunMessage("error occured when read msg from robot!",Qt::red);
    }
    //    addRunMessage("robot "+QString::number(robotNumber,10)+" received a msg:"+msg,Qt::blue);
    processMsgFromRobot(robotNumber,msg);
    // updateUI();



//    std::vector<QString> msgArray;
//    if(!robotSocketArray[robotNumber]->readMsg(msgArray))
//    {
//        slotRobotOffLine(robotNumber);
//        addRunMessage("error occured when read msg from robot!",Qt::red);
//    }
//    //    addRunMessage("robot "+QString::number(robotNumber,10)+" received a msg:"+msg,Qt::blue);
//    for(uint i=0;i<msgArray.size();++i){
//        processMsgFromRobot(robotNumber,msgArray[i]);
//    }

//    updateUI();

}

void SchedulerGUI::slotDisconnect(uint robotNumber)
{
    addRunMessage("robot "+QString::number(robotNumber,10)+" disconnect from server!",Qt::red);
    slotRobotOffLine(robotNumber);
}

void SchedulerGUI::slotSendMsg(uint robotNumber,QString msg)
{
    if(!robotSocketArray[robotNumber]->sendMsg(msg))
    {
        slotRobotOffLine(robotNumber);
        addRunMessage(QString::number(robotNumber,10)+" error occuered when send msg to robot!",Qt::red);
    }
}

void SchedulerGUI::checkCatchable()
{
    ///////////confirm which robot can catch adapter///////////////////////////////////////
    if(
            (!isValidModule)
            ||detectAdapterTime>maxDetectAdapterTime
            ||detectAdapterTime==0
            ||isEmergencyStop
            )
        return;


    bool isSendCatchOrderNow=true;
    std::vector<uint>nowCanCatchRobotArray;
    ///calculate how much robot can catch now///////
    for(uint i=0;i<robotNum&&i!=ultrasonicIndex;++i)
    {
        if((moduleStateArray[robotModuleNumber[i]]==MODULE_ULTRASONIC||
            moduleStateArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                &&robotWorkStateArray[i]==ROBOT_WAITING
                )
        {
            nowCanCatchRobotArray.push_back(i);
        }
    }

    if((!isUltraValid)&&isConveyorRunning){
        calculateOptimizeCatchTime(isSendCatchOrderNow,nowCanCatchRobotArray);
    }


    if(isUltraValid)
        checkUltrasonicIsCatchable();

    if(nowCanCatchRobotArray.size()>0&&isSendCatchOrderNow)
    {
        stopConveyor();
        for(uint i=0;i<nowCanCatchRobotArray.size();++i)
        {
            sendCatchOrder(nowCanCatchRobotArray[i]);
        }
    }
}

bool SchedulerGUI::checkUltrasonicIsCatchable()
{
    bool isCanCatch=false;
    bool isCanInsert=false;

    if(robotWorkStateArray[ultrasonicIndex]==ROBOT_WAITING&&
            moduleStateArray[robotModuleNumber[ultrasonicIndex]]==MODULE_LOADED)
    {
        isCanCatch=true;
    }
    if(robotWorkStateArray[ultrasonicIndex]==ROBOT_WAITING&&
            moduleStateArray[robotModuleNumber[ultrasonicIndex]+1]==MODULE_ULTRA1_CAUGHT)
    {
        isCanInsert=true;
    }
    QString order="";
    if(isCanCatch)
        order+="catch";
    if(isCanInsert)
        order+="insert";
    uint ultraRobotWorkState;
    if(isCanCatch&&isCanInsert)
    {
        ultraRobotWorkState=ULTRA_ROBOT_ACK_INSERT_CATCH;
        // moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_CATCHING;
        slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_CATCHING);
        robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_INSERT_CATCH);
    }
    else if(isCanCatch)
    {
        ultraRobotWorkState=ULTRA_ROBOT_ACK_CATCH;
        // moduleStateArray[robotModuleNumber[ultrasonicIndex]]=MODULE_CATCHING;
        slotModuleStateChanged(robotModuleNumber[ultrasonicIndex],MODULE_CATCHING);
        robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_CATCH);
    }
    else if(isCanInsert)
    {
        ultraRobotWorkState=ULTRA_ROBOT_ACK_INSERT;
        robotMsgTimer[ultrasonicIndex]->setTimerData(ultrasonicIndex,ULTRA_ROBOT_ACK_INSERT);
    }
    if(isCanCatch||isCanInsert)
    {
//        sendOrderToUltrasonicRobot(order);
        ultraThread->ultraServer.sendMsg(order);
        slotUltraRobotWorking(ultraRobotWorkState);
    }

    return isCanCatch&&isCanInsert;
}

void SchedulerGUI::calculateOptimizeCatchTime(bool &isCatchNow, std::vector<uint> &nowCanCatchRobotArray)
{
    std::vector<uint>nextCanCatchRobotArray;
    std::vector<uint>nNextCanCatchRobotArray;
    std::vector<uint>nnNextCanCatchRobotArray;
    std::vector<uint>nnnNextCanCatchRobotArray;

    ////calculate the next time how many robot can catch////
    uint* nextModuleStatusArray=new uint[moduleNum];
    uint lastModuleState=moduleStateArray[moduleNum-1];
    for(uint i=moduleNum-1;i>0;--i)
    {
        nextModuleStatusArray[i]=moduleStateArray[i-1];
    }
    nextModuleStatusArray[0]=lastModuleState;

    for(uint i=0;i<robotNum;++i)
    {
        if(
                (nextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                 ||nextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                )
        {
            nextCanCatchRobotArray.push_back(i);
        }
    }
    delete[] nextModuleStatusArray;


    //////////////////////calculate the next next module status array////////////////////////////////

    uint* nNextModuleStatusArray=new uint[moduleNum];
    uint nn_lastModuleState=moduleStateArray[moduleNum-1];
    uint nn__lastModuleState=moduleStateArray[moduleNum-2];

    for(uint i=moduleNum-1;i>1;--i)
    {
        nNextModuleStatusArray[i]=moduleStateArray[i-2];
    }
    nNextModuleStatusArray[0]=nn__lastModuleState;
    nNextModuleStatusArray[1]=nn_lastModuleState;
    ////calculate the next next time how many robot can catch////
    for(uint i=0;i<robotNum;++i)
    {
        if(
                (nNextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                 ||nNextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                )
        {
            nNextCanCatchRobotArray.push_back(i);
        }
    }
    delete[] nNextModuleStatusArray;

    //////////////////////calculate the next next next module status array////////////////////////////////
    uint* nnNextModuleStatusArray=new uint[moduleNum];
    uint nnn_lastModuleState=moduleStateArray[moduleNum-1];
    uint nnn__lastModuleState=moduleStateArray[moduleNum-2];
    uint nnn___lastModuleState=moduleStateArray[moduleNum-3];
    for(uint i=moduleNum-1;i>2;--i)
    {
        nnNextModuleStatusArray[i]=moduleStateArray[i-3];
    }
    nnNextModuleStatusArray[0]=nnn___lastModuleState;
    nnNextModuleStatusArray[1]=nnn__lastModuleState;
    nnNextModuleStatusArray[2]=nnn_lastModuleState;
    ////calculate the next next next time how many robot can catch////
    for(uint i=0;i<robotNum;++i)
    {
        if(
                (nnNextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                 ||nnNextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                )
        {
            nnnNextCanCatchRobotArray.push_back(i);
        }
    }
    delete[] nnNextModuleStatusArray;




    //////////////////////calculate the next next next next module status array////////////////////////////////

    uint* nnnNextModuleStatusArray=new uint[moduleNum];
    uint nnnn_lastModuleState=moduleStateArray[moduleNum-1];
    uint nnnn__lastModuleState=moduleStateArray[moduleNum-2];
    uint nnnn___lastModuleState=moduleStateArray[moduleNum-3];
    uint nnnn____lastModuleState=moduleStateArray[moduleNum-4];
    for(uint i=moduleNum-1;i>3;--i)
    {
        nnnNextModuleStatusArray[i]=moduleStateArray[i-4];
    }
    nnnNextModuleStatusArray[0]=nnnn____lastModuleState;
    nnnNextModuleStatusArray[1]=nnnn___lastModuleState;
    nnnNextModuleStatusArray[2]=nnnn__lastModuleState;
    nnnNextModuleStatusArray[3]=nnnn_lastModuleState;
    ////calculate the next next next time how many robot can catch////
    for(uint i=0;i<robotNum;++i)
    {
        if(
                (nnnNextModuleStatusArray[robotModuleNumber[i]]==MODULE_LOADED
                 ||nnnNextModuleStatusArray[robotModuleNumber[i]]==MODULE_UNRECOGNIZE_ONCETIME)
                &&(robotWorkStateArray[i]==ROBOT_WAITING||robotWorkStateArray[i]==ROBOT_RECOGNIZE_ERROR)
                )
        {
            nnNextCanCatchRobotArray.push_back(i);
        }
    }
    delete[] nnnNextModuleStatusArray;


    if(
            (nextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            ||(nNextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            ||(nnNextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            ||(nnnNextCanCatchRobotArray.size()>nowCanCatchRobotArray.size())
            )
    {
        isCatchNow=false;
    }
}

void SchedulerGUI::processMsgFromRobot(uint robotNumber,QString msg)
{
    // "wait" ROBOT_WAITING
    // "catched" ROBOT_CATCHED
    // "unrecognize" ROBOT_UNRECOGNIZE
    // "clear" ROBOT_CLEARING
    // "ur1_unrecognized" ULTRASONIC_ROBOT1_UNRECOGNIZED
    // "ur2_unrecognized" ULTRASONIC_ROBOT2_UNRECOGNIZE
    // "ur1_caught" ULTRASONIC_ROBOT1_CATCHED
    // "ur2_inserted" ULTRASONIC_ROBOT2_INSERTED
    // "ur1_insert_error" ULTRASONIC_ROBOT2_INSERT_ERROR
    // "ur1_inserted-wait" ULTRASONIC_ROBOT2_INSERTED_WAIT
//    if(robotNumber==ultrasonicIndex)
//    {
//       addRunMessage("receive message from ultra:"+msg,Qt::blue);
//    }

    if(robotMsgToStatusArray.find(msg)==robotMsgToStatusArray.end())
    {
        addRunMessage("robot "+QString::number(robotNumber,10)+" receive a error message:"+msg,Qt::red);
        return;
    }
    switch (robotMsgToStatusArray[msg]){
    case ROBOT_WAITING:
    {
        slotRobotWaiting(robotNumber);
    }
        break;
    case ROBOT_CATCHED:
    {
        slotRobotCatched(robotNumber);
    }
        break;
    case ROBOT_RECOGNIZE_ERROR:
    {
        slotTestRobotRecognizeError(robotNumber);
    }
        break;
    case ROBOT_CLEARING:
    {
        slotTestRobotClearing(robotNumber);
    }
        break;
    case ULTRASONIC_ROBOT1_UNRECOGNIZED:
    {
        slotUltraRobot1Unrecognized();
    }
        break;
    case ULTRASONIC_ROBOT2_UNRECOGNIZED:
    {
        slotUltraRobot2Unrecognized();
    }
        break;
    case ULTRASONIC_ROBOT2_INSERTED:
    {
        slotUltraRobot2Inserted();
    }
        break;
    case ULTRASONIC_ROBOT1_CATCH_ERROR:
    {
        slotUltraRobot1CatchError();
    }
        break;
    case ULTRASONIC_ROBOT2_INSERT_ERROR:
    {
        slotUltraRobot2InsertError();
    }
        break;
    case ULTRASONIC_ROBOT2_INSERTED_WAIT:
    {
        slotUltraRobot2Inserted();
        slotRobotWaiting(robotNumber);
    }
        break;
    case ULTRASONIC_ROBOT2_CAUGHT_WAIT:
    {
        slotUltraRobot1Caught();
        slotRobotWaiting(robotNumber);
    }
        break;
    default:
    {
        addRunMessage("robot "+QString::number(robotNumber,10)+" receive a error message:"+msg,Qt::red);
    }
        break;
    }
}

void SchedulerGUI::statisticAdapter(uint robotNumber,uint flag)
{
    if(flag==ROBOT_RECOGNIZE_ERROR)
    {
        robotUnrecognizeNum[robotNumber]++;
        robotUnrecognizeNumLineEdit[robotNumber]->setText(QString::number(robotUnrecognizeNum[robotNumber],10));
    }
    if(flag==ROBOT_CATCHED)
    {
        caughtNum[robotNumber]++;
        robotCaughtNumLineEdit[robotNumber]->setText(QString::number(caughtNum[robotNumber],10));
    }
}

void SchedulerGUI::testRobotUnrecognized(uint robotNumber)
{
    slotRobotStateChanged(robotNumber,ROBOT_RECOGNIZE_ERROR);
    if(moduleStateArray[robotModuleNumber[robotNumber]]==MODULE_UNRECOGNIZE_ONCETIME)
        slotModuleStateChanged(robotModuleNumber[robotNumber],MODULE_UNRECOGNIZE_TWICETIME);
    else
        slotModuleStateChanged(robotModuleNumber[robotNumber],MODULE_UNRECOGNIZE_ONCETIME);
    addRunMessage("robot "+QString::number(robotNumber,10)+" unrecognize",Qt::red);
}

void SchedulerGUI::on_pushButton_clicked()
{
//    ultraServer->runServer();
    if(checkCatchableTimer.isActive()){
        if(QMessageBox::Yes==QMessageBox::question(NULL,"Warning","Scheduler is running,open this will restore the scheduler's status?",QMessageBox::Yes,QMessageBox::No))
        {
            bool isOK=false;
            QString str=QInputDialog::getText(NULL,
                                              "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                              QLineEdit::Password,
                                              QDir::home().dirName(),&isOK);
            if(str=="qwer1234"){
                checkCatchableTimer.stop();
                checkResetSignalTimer.stop();
                stopConveyor();

                ///////////restore the module status array///////////
                for(uint i=0;i<moduleNum;++i){
                    moduleStateArray[i]=MODULE_EMPTY;
                }
                ///////////restore the robot work status array///////
                closeServer();
                ioUI->show();
                addRunMessage("Open the io control interface!",Qt::red);
                ui->label_7->setStyleSheet("background:red");
            }
            else if(isOK)
            {
                QMessageBox::warning(NULL,"Warning","password error!");
            }
        }
    }
    else
        ioUI->show();
}

void SchedulerGUI::addRunMessage(QString msg,const QColor&color)
{
    ui->textEdit->setTextColor(color);
    ui->textEdit->append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+"   "+ msg);
}

void SchedulerGUI::robotReplyMsgTimeOut(const uint &robotNumber)
{
    uint timerDescriptor;
    timerDescriptor=robotMsgTimer[robotNumber]->getTimerDescriptor();
    switch(timerDescriptor){
    case ROBOT_ACK_CATCHED:
        slotTestRobotAckCatchedError(robotNumber);
        break;
    case ULTRA_ROBOT_ACK_INSERT_CATCH:
        slotUltraRobotAckError(ULTRA_ROBOT_ACK_INSERT_CATCH);
        break;
    case ULTRA_ROBOT_ACK_INSERT:
        slotUltraRobotAckError(ULTRA_ROBOT_ACK_INSERT);
        break;
    case ULTRA_ROBOT_ACK_CATCH:
        slotUltraRobotAckError(ULTRA_ROBOT_ACK_CATCH);
        break;
    default:{

    }
        break;
    }
}

void SchedulerGUI::on_pushButton_3_clicked()
{
    clearError();
}

void SchedulerGUI::on_radioButton_clicked()
{
    if(!_isProductiveTwoModel)
    {
        int choose;
        choose=QMessageBox::question(NULL,"Warning","Are you sure to switch the model of product?",QMessageBox::Yes,QMessageBox::No);
        if(choose==QMessageBox::Yes)
        {
            bool isOK=false;
            QString str=QInputDialog::getText(NULL,
                                              "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                              QLineEdit::Password,
                                              QDir::home().dirName(),&isOK);
            if(str=="qwer1234"){
                QMessageBox::warning(NULL,"Warning","You need to restart the program in order to activate the change!");
                QString configFilePath="settings.ini";
                QSettings settings(configFilePath,QSettings::IniFormat);
                settings.setValue("runData/isProductiveTwoModel",true);
                _isProductiveTwoModel=true;
            }
            else if(isOK){
                QMessageBox::warning(NULL,"Warning","Password error!");
                ui->radioButton_2->setChecked(true);
            }
        }
        else
            ui->radioButton_2->setChecked(true);
    }
}

void SchedulerGUI::on_radioButton_2_clicked()
{
    if(_isProductiveTwoModel)
    {
        int choose;
        choose=QMessageBox::question(NULL,"Warning","Are you sure to switch the model of product?",QMessageBox::Yes,QMessageBox::No);
        if(choose==QMessageBox::Yes)
        {
            bool isOK=false;
            QString str=QInputDialog::getText(NULL,
                                              "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                              QLineEdit::Password,
                                              QDir::home().dirName(),&isOK);
            if(str=="qwer1234"){
                QMessageBox::warning(NULL,"Warning","You need to restart the program in order to activate the change!");
                QString configFilePath="settings.ini";
                QSettings settings(configFilePath,QSettings::IniFormat);
                settings.setValue("runData/isProductiveTwoModel",false);
                _isProductiveTwoModel=false;
            }
            else if(isOK){
                QMessageBox::warning(NULL,"Warning","Password error!");
                ui->radioButton->setChecked(true);
            }
        }
        else
            ui->radioButton->setChecked(true);
    }
}

void SchedulerGUI::on_pushButton_4_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Reset the conveyor?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            addRunMessage("Reset conveyor!",Qt::red);
            if(isProductiveTwoModel)
            {
                isValidModule=false;
                isStartCount=false;
            }
            for(uint i=0;i<moduleNum;++i)
            {
                if(moduleStateArray[i]==MODULE_ULTRASONIC)
                    moduleStateArray[i]=MODULE_EMPTY;
            }
            updateModuleUI();
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
        }
    }
}

void SchedulerGUI::on_pushButton_2_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot A?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            slotRobotOffLine(0);
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            //            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_5_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot B?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            slotRobotOffLine(1);
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            //            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_6_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with robot C?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            slotRobotOffLine(2);
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            //            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_pushButton_7_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Close the connection with ultrasonic robot?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            slotRobotOffLine(ultrasonicIndex);
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            //            ui->radioButton->setChecked(true);
        }
    }
}

void SchedulerGUI::on_checkBox_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Change the Ultrasonic state?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            addRunMessage("Change Ultrasonic State!",Qt::red);
            isUltraValid=ui->checkBox->isChecked();
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->checkBox->setChecked(isUltraValid);
        }
        else
            ui->checkBox->setChecked(isUltraValid);
    }
    else
        ui->checkBox->setChecked(isUltraValid);
}

void SchedulerGUI::on_checkBox_2_clicked()
{
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Change the last robot state?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            addRunMessage("Change last robot State!",Qt::red);
            isStopLastRobot=ui->checkBox_2->isChecked();
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->checkBox_2->setChecked(isStopLastRobot);
        }
        else
            ui->checkBox_2->setChecked(isStopLastRobot);
    }
    else
        ui->checkBox_2->setChecked(isStopLastRobot);
}

void SchedulerGUI::on_checkBox_3_clicked()
{
//    isOnlyCatch=ui->checkBox_3->isChecked();
//    qDebug()<<isOnlyCatch;
    int choose;
    choose=QMessageBox::question(NULL,"Warning","Change the product mode?",QMessageBox::Yes,QMessageBox::No);
    if(choose==QMessageBox::Yes)
    {
        bool isOK=false;
        QString str=QInputDialog::getText(NULL,
                                          "Warning","Please input password to confirm this operation(PASSWORD:qwer1234)",
                                          QLineEdit::Password,
                                          QDir::home().dirName(),&isOK);
        if(str=="qwer1234")
        {
            addRunMessage("Change the product mode!",Qt::red);
            isOnlyCatch=ui->checkBox_3->isChecked();
        }
        else if(isOK)
        {
            QMessageBox::warning(NULL,"Warning","Password error!");
            ui->checkBox_3->setChecked(isOnlyCatch);
        }
        else
            ui->checkBox_3->setChecked(isOnlyCatch);
    }
    else
        ui->checkBox_3->setChecked(isOnlyCatch);
}
