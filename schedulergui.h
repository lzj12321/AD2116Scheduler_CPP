#ifndef SCHEDULERGUI_H
#define SCHEDULERGUI_H

#include <QMainWindow>
#include<QLabel>
#include<QLineEdit>
#include<map>
#include<io_control.h>
#include<QTime>
#include<QTcpServer>
#include<QTcpSocket>
#include<QTimer>
#include<gpio_pi.h>
#include<socket.h>
#include<scheduler.h>
#include<QCloseEvent>
#include<timer.h>
#include<QEvent>
#include<ultrasonicserver.h>
#include<QThread>
#include<serverthread.h>

using namespace Scheduler;

namespace Ui {
class SchedulerGUI;
}

class SchedulerGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit SchedulerGUI(QWidget *parent = 0);
    ~SchedulerGUI();
    void closeEvent(QCloseEvent *event);
private:
    void uiIni();
    void dataIni();
    void paramIni();
    bool IoIni();
    void deviceIni();
    bool serverIni();
    void ultrasonicServerIni();

    void updateUI();
    void updateModuleUI();
    void updateRobotUI();
    void updateSingleModule(uint moduleIndex,uint state);
    void updateSingleRobot(uint robotIndex,uint state);
    void setRobotLabelPosition();
    void iniModuleLabel();
    void connectSlotAndSignal();
    void closeServer();

    void sendOrderToUltrasonicRobot(QString order);
    void sendCatchOrder(uint robotNumber);
    void sendClearOrder(uint robotNumber);
    void emergencyStop();
    void stopConveyor();
    void activateConveyor();
    void removeRobotSocket(uint robotNumber);
    void addRobotSocket(uint robotNumber,Socket*);
    void processMsgFromRobot(uint robotNumber,QString msg);
    void statisticAdapter(uint robotNumber,uint flag);
    void testRobotUnrecognized(uint robotNumber);
signals:
    void signalAddAdapter(uint modelState);
private slots:
    void slotRobotWaiting(uint robotNumber);
    void slotRobotCatching(uint robotNumber);
    void slotRobotCatched(uint robotNumber);
    void slotRobotOnLine(uint robotNumber);
    void slotRobotOffLine(uint robotNumber);

    void slotTestRobotAckCatchedError(uint robotNumber);
    void slotTestRobotRecognizeError(uint robotNumber);
    void slotTestRobotClearing(uint robotNumber);

    void slotUltrasonicRobotOnline();
    void slotUltrasonicRobotOffLine();
    void slotUltraRobotWorking(uint workState);
    void slotUltraRobot1Caught();
    void slotUltraRobot2Inserted();
    void slotUltraRobot1Unrecognized();
    void slotUltraRobot2Unrecognized();
    void slotUltraRobotAckError(uint);
    void slotUltraRobot2InsertError();
    void slotUltraRobot1CatchError();
    void slotReceivedMsgFromUltra(QString);
    void slotUltraServerIniError();
    void slotRobotStateChanged(uint robotIndex,uint robotState);
    void slotModuleStateChanged(uint moduleIndex,uint moduleState);

    void slotAddAdapter(uint modelState);
    void checkCatchable();
    bool checkUltrasonicIsCatchable();
    void calculateOptimizeCatchTime(bool &isCatchNow, std::vector<uint> &nowCanCatchRobotArray);
    void updateModuleArrayState();
    void checkIsActivateConveyor();
    void clearError();
    void clearAckCatchedError();
    void clearLastModuleHaveAdapterError();
    void checkResetButtonStatus();
    void slotControlPlate(uint,uint);
    void slotSetConveyorStatus(uint,uint);
    void slotReleaseAllPlate();
    void slotPressAllPlate();
    void slotSetAlarmStatus(bool,uint alarmFlag);
    void slotAlarmTimerTimeout();

    void slotNewConnection();
    void slotReceiveMsg(uint);
    void slotDisconnect(uint);
    void slotSendMsg(uint,QString);
    void addRunMessage(QString msg,const QColor&color);
    void robotReplyMsgTimeOut(const uint&robotNumber);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
    void on_radioButton_clicked();
    void on_radioButton_2_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_checkBox_clicked();
    void on_checkBox_2_clicked();
    void on_checkBox_3_clicked();

private:
    uint* robotUnrecognizeNum;
    uint* caughtNum;

    Ui::SchedulerGUI *ui;
    IO_Control* ioUI;

    int _time=0;

    QTimer checkCatchableTimer;

    ////////1.ack catched timeout  2.clear last loaded module///////////////
    QTimer checkResetSignalTimer;
    QTimer alarmTimer;

    uint checkCatchTimerInterval;
    uint checkResetButtonInterval;
    uint flashAlarmInterval;

    bool isConveyorRunning=false;
    bool isStartCount=false;
    bool isValidModule=true;
    bool isProductiveTwoModel=false;
    bool _isProductiveTwoModel=false;
    bool alarmStatus=false;

    ///////detect adapter about/////////
    bool prevR4AdapterSensorStatus=false;
    bool prevR1ModuleSensorStatus=false;
    bool prevR1AdapterSensorStatus=false;
    uint detectAdapterTime=0;
    uint maxDetectAdapterTime=4;
    bool isDetectFallingEdge=false;
    bool isHaveAdapter=false;
    bool isEmergencyStop=false;
    bool isPushResetButton=false;
    bool isOnlyCatch=false;

    QString catchOrder,clearOrder,dropOrder;

    uint moduleNum;
    uint robotNum;
    uint lineModuleNum=0;
    uint* robotWorkStateArray;
    uint* moduleStateArray;
    uint checkCatchingMsgInterval=500;
    uint checkCatchedMsgInterval=7000;

    int lastOnlineTestRobotNumber=-1;
    int onlineTestRobotNum=0;
    bool isUltrasonicOnline=false;
    //    int onlineRobotNum=0;
    uint detectSensorPosition=0;
    uint ultrasonicIndex=3;

    uint modelGap=20;
    uint modelRadius=40;
    uint labelGap=60;
    uint robotLabelWidth=230;
    uint robotLabelHeight=50;

    QLabel** robotLabelArr;
    QLabel* conveyorLabel;
    QLabel** moduleLabelArray;
    QLineEdit** robotUnrecognizeNumLineEdit;
    QLineEdit** robotCaughtNumLineEdit;

    uint conveyorStatusIO,conveyorDirectionIO;
    uint alarmStatusIO,resetAlarmButtonIO;

    bool flag=false;

    std::map<uint,QString> robotName;
    std::map<uint,uint> robotModuleNumber;
    std::map<uint,QString>robotIP;
    std::map<uint,uint>robotPlateIO;
    std::map<uint,uint>robotModuleSensorIO;
    std::map<uint,uint>robotAdapterSensorIO;
    std::map<uint,Socket*>robotSocketArray;
    std::map<QString,uint>robotMsgToStatusArray;
    std::map<uint,Timer*>robotMsgTimer;

    QTcpServer* server;
    Timer* timerPtr;
    GPIO_PI gpio_pi;

    bool isUltraValid=true;
    bool isStopLastRobot=true;

    ServerThread* ultraThread;
//    UltrasonicServer* ultraServer;
};

#endif // SCHEDULERGUI_H
