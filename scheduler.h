#ifndef SCHEDULER_H
#define SCHEDULER_H

namespace Scheduler {

#define CONVEYOR_ACTIVATE 0
#define CONVEYOR_STOP 1
#define CONVEYOR_FORWARD 1
#define CONVEYOR_REVERSE 0


#define PLATE_PRESS 0
#define PLATE_RELEASE 1


#define START_ALARM 0
#define STOP_ALARM 1
#define ALARM_CONTINUOUS 0
#define ALARM_FLASH 1


#define ROBOT_WAITING 0
#define ROBOT_OFFLINE 2
#define ROBOT_CATCHED 4
#define ROBOT_ACK_CATCHED_ERROR 7
#define ROBOT_ACK_CATCHED 9
#define ROBOT_RECOGNIZE_ERROR 10
#define ROBOT_CONNECTED 11
#define ROBOT_DROP_PASS 12
#define ROBOT_CLEARING 13
#define ULTRASONIC_ROBOT1_UNRECOGNIZED 1
#define ULTRASONIC_ROBOT2_UNRECOGNIZED 3
#define ULTRASONIC_ROBOT2_INSERTED 6
#define ULTRA_ROBOT_ACK_INSERT_CATCH 14
#define ULTRA_ROBOT_ACK_INSERT 15
#define ULTRA_ROBOT_ACK_CATCH 16
#define ULTRA_ROBOT_ACK_CATCH_ERROR 17
#define ULTRA_ROBOT_ACK_INSERT_ERROR 18
#define ULTRA_ROBOT_ACK_CATCH_INSERT_ERROR 19
#define ULTRA_ROBOT_WORKING 20
#define ULTRASONIC_ROBOT2_INSERT_ERROR 21
#define ULTRASONIC_ROBOT1_CATCH_ERROR 24
#define ULTRASONIC_ROBOT2_INSERTED_WAIT 22
#define ULTRASONIC_ROBOT2_CAUGHT_WAIT 23


#define MODULE_EMPTY 0
#define MODULE_LOADED 1
#define MODULE_CATCHING 2
#define MODULE_ULTRA1_CAUGHT 3
#define MODULE_ULTRA1_UNRECOGNIZED 4
#define MODULE_ULTRA2_UNRECOGNIZED 9
#define MODULE_ERROR 5
#define MODULE_UNRECOGNIZE_ONCETIME 6
#define MODULE_UNRECOGNIZE_TWICETIME 7
#define MODULE_ULTRASONIC 8
}

#endif // SCHEDULER_H
