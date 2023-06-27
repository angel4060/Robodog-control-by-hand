/*****************************************************************
 Copyright (c) 2020, Unitree Robotics.Co.Ltd. All rights reserved.
******************************************************************/

#include "unitree_legged_sdk/unitree_legged_sdk.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>

using namespace UNITREE_LEGGED_SDK;

std::ifstream myfile;

class Custom
{
public:
    Custom(uint8_t level): 
      safe(LeggedType::Go1), 
      udp(level, 8090, "192.168.12.1", 8082){
        udp.InitCmdData(cmd);
    }
    void UDPRecv();
    void UDPSend();
    void RobotControl();
    
    Safety safe;
    UDP udp;
    HighCmd cmd = {0};
    HighState state = {0};
    uint8_t walk_flag=0;
    int motiontime = 0;
    float dt = 0.002;     // 0.001~0.01
    char key;
    char command;
    std::ifstream myfile;
    
};


void Custom::UDPRecv()
{
    udp.Recv();
}

void Custom::UDPSend()
{  
    udp.Send();
}

void Custom::RobotControl() 
{
    // motiontime += 1;
    udp.GetRecv(state);
    printf("%d   %f\n", motiontime, state.imu.quaternion[2]);
 
    myfile.open("command.txt");
    cmd.mode = 0;      /*
                          mode   0 : idle, default stand - โหมดปกติ หยุดเคลื่อนที่
                                 1 : forced stand - หยุดนิ่งและควบคุมสูงตัว     
                                 2 : target velocity walking (controlled by velocity + yawSpeed) - กำหนดความเร็วและความเร็วการหมุนที่ต้องการเดิน
                                 3 : position walk goto that position [0]=x [1]=y reference from dog frame at first time run - กำหนดตำแหน่งที่ต้องการไป
                                 4 : path mode walking (reserve for future release) - ยังไม่สามารถใช้งานได้ 
                                 5 : position stand down - ยกตัวลงจากท่ายืน
                                 6 : position stand up - ยกตัวขึ้นจากท่ายืน
                                 7 : damping mode - ลดการสั่นสะเทือน
                                 8 : recovery stand - กลับสู่ท่ายืนหลังจากล้มลง
                                 9 : backflip - ทำการกลับหลังจากกระโดด
                                 10 : jumpYaw - กระโดดโดยที่หันหน้าไปที่ทิศทางที่กำหนด
                                 11 : straightHand - ยกมือขึ้น
                                 12 : dance1 - เต้นรำ 1
                                 13 : dance2 - เต้นรำ 2
                       */ 
    cmd.gaitType = 0;           // 0.idle  1.trot  2.trot running  3.climb stair  4.trot obstacle
    cmd.speedLevel = 0;         // 0. default low speed. 1. medium speed 2. high speed. during walking, only respond MODE 3
    cmd.footRaiseHeight = 0;    // (unit: m, default: 0.08m), foot up height while walking, delta value
    cmd.bodyHeight = 0;         // (unit: m, default: 0.28m), delta value
    cmd.euler[0] = 0;           // (unit: rad), roll in stand mode
    cmd.euler[1] = 0;           // (unit: rad), pitch in stand mode
    cmd.euler[2] = 0;           // (unit: rad), yaw in stand mode
    cmd.velocity[0] = 0.0f;     // velocity[0] = forward 
    cmd.velocity[1] = 0.0f;     // velocity[1] = sideward
    cmd.position[0] = 0.0f;     // position[0] = x
    cmd.position[1] = 0.0f;     // position[1] = y
    cmd.yawSpeed = 0.0f;        // yawspeed > 0 , yawspeed = turn , yawspeed < 0 = turn , yawspeed == 0 = straight 
    cmd.reserve = 0;            // none use
    
//----------------------------------------------------------------------------------------------------------------------------//
    myfile >> command;
    
    std::cout << command << std::endl;

    if (command == 98) // 'b' 
    {
        std::cout << "stand" << std::endl;
        // cmd.mode = 1;
        // cmd.bodyHeight = 0.0;
    }
    else if (command == 103) // 'g'
    {
        std::cout << "sit" << std::endl;
        cmd.mode = 1;
        cmd.bodyHeight = -0.5f;
    }
    else if (command == 102) // 'f'
    {
        std::cout << "go forward" << std::endl;
        cmd.mode = 2; // target velocity walking
        cmd.gaitType = 1; // trot
        cmd.velocity[0] = 0.5f; // -1  ~ +1
        cmd.footRaiseHeight = 0.3;
    }
    else if (command == 100) // 'd'
    {
        std::cout << "backward" << std::endl;
        cmd.mode = 2;
        cmd.gaitType = 1;
        cmd.velocity[0] = -0.5f; // -1  ~ +1
        cmd.yawSpeed = 0;
        cmd.footRaiseHeight = 0.3;
    }
    else if (command == 108) // 'l'
    {
        std::cout << "go left" << std::endl;
        cmd.mode = 2; // target velocity walking
        cmd.gaitType = 1; // trot
        cmd.velocity[1] = 0.5f; // -1  ~ +1
        cmd.footRaiseHeight = 0.3;
    }
    else if (command == 114) // 'r'
    {
        std::cout << "go right" << std::endl;
        cmd.mode = 2; // target velocity walking
        cmd.gaitType = 1; // trot
        cmd.velocity[1] = -0.5f; // -1  ~ +1
        cmd.footRaiseHeight = 0.3;
    }
    else if (command == 76) // 'L'
    {
        std::cout << "turn left" << std::endl;
        cmd.mode = 2; // target velocity walking
        cmd.gaitType = 1; // trot
        cmd.yawSpeed = -1;
        cmd.footRaiseHeight = 0.3;
    }
    else if (command == 82) // 'R'
    {
        std::cout << "turn right" << std::endl;
        cmd.mode = 2; // target velocity walking
        cmd.gaitType = 1; // trot
        cmd.yawSpeed = 1;
        cmd.footRaiseHeight = 0.3;
    }

//----------------------------------------------------------------------------------------------------------------------------//

    else if (command == 115) // 's'
    {
        std::cout << "straightHand" << std::endl;

        cmd.mode = 11;
    }
    else if (command == 69) // 'E'
    {
        std::cout << "dance1" << std::endl;
        cmd.mode = 12;
    }
    else if (command == 101) // 'e'
    {
        std::cout << "dance2" << std::endl;
        cmd.mode = 13;
    }

//----------------------------------------------------------------------------------------------------------------------------//

// Set command 
// Not Stable
//----------------------------------------------------------------------------------------------------------------------------//

    // else if (command == 10) // '?'
    // {
    //     std::cout << "set1" << std::endl; 
    //     motiontime = 0;
    //     while(motiontime <= 3000)
    //     {
    //         motiontime += 1;
    //         if (motiontime >= 0 && motiontime <= 500) // Forward
    //         {
    //             cmd.mode = 2;
    //             cmd.gaitType = 1;
    //             cmd.velocity[0] = 0.5f; // -1  ~ +1
    //             cmd.yawSpeed = 0;
    //             cmd.footRaiseHeight = 0.3; 
    //         }
    //         else if (motiontime > 1000 && motiontime <= 4500) // straightHand
    //         {
    //             cmd.mode = 11;
    //         }
    //         else if (motiontime > 4500 && motiontime <= 5500) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 5500 && motiontime <= 9000) // straightHand
    //         {
    //             cmd.mode = 11;
    //         }
    //         else if (motiontime > 9000 && motiontime <= 10000) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 10000 && motiontime <= 11000) // Stand
    //         {
    //             cmd.mode = 1;
    //             cmd.bodyHeight = 0.0;
    //         }
    //         else if (motiontime > 11000 && motiontime <= 12000) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 12000 && motiontime <= 16000) // straightHand
    //         {
    //             cmd.mode = 11;
    //         }
    //         else if (motiontime > 16000 && motiontime <= 17000) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 17000 && motiontime <= 17500) // Backward
    //         {
    //             cmd.mode = 2;
    //             cmd.gaitType = 1;
    //             cmd.velocity[0] = -0.5f; // -1  ~ +1
    //             cmd.footRaiseHeight = 0.3;
    //         }
    //         else if (motiontime > 17500 && motiontime <= 18000) // Stand
    //         {
    //             cmd.mode = 1;
    //             cmd.bodyHeight = 0.0;
    //         }
    //     }
        
    // }
    // else if (command == 10) // '?'
    // {
    //     std::cout << "set2" << std::endl;
    //     motiontime = 0;
    //     while(motiontime <= 11000)
    //     {
    //         motiontime += 1;
    //         if (motiontime >= 0 && motiontime <= 1000) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 1000 && motiontime <= 10000) // dance1
    //         {
    //             cmd.mode = 12;
    //         }
    //         else if (motiontime > 10000 && motiontime <= 10500) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 10500 && motiontime <= 11000) // Stand
    //         {
    //             cmd.mode = 1;
    //             cmd.bodyHeight = 0.0;
    //         }
    //         else if (motiontime > 11000 && motiontime <= 12000) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //         else if (motiontime > 12000 && motiontime <= 15000) // dance2
    //         {
    //             cmd.mode = 13;
    //         }
    //         else if (motiontime > 31000 && motiontime <= 32000) // jumpYaw
    //         {
    //             cmd.mode = 10;
    //         }
    //     }
        
    //}

//----------------------------------------------------------------------------------------------------------------------------//


// Test Function
// Run by use motiontime to count time 
//----------------------------------------------------------------------------------------------------------------------------//

    // if (motiontime >= 0 && motiontime <= 1000) // Stand
    // {
    //     cmd.mode = 1;
    //     cmd.bodyHeight = 0.0;
    // }
    // else if (motiontime > 1000 && motiontime <= 2000) // Forward
    // {
    //     cmd.mode = 2;
    //     cmd.gaitType = 1;
    //     cmd.velocity[0] = 0.5f; // -1  ~ +1
    //     cmd.yawSpeed = 0;
    //     cmd.footRaiseHeight = 0.3;
    // }
    // else if (motiontime > 2000 && motiontime <= 3000) // Turn Right
    // {
    //     cmd.mode = 2;
    //     cmd.gaitType = 1;
    //     //cmd.velocity[0] = 0.0f; // -1  ~ +1
    //     cmd.velocity[1] = 0.5f; // -1  ~ +1
    //     //cmd.yawSpeed = -1;
    //     cmd.footRaiseHeight = 0.3;
    // }
    // else if (motiontime > 3000 && motiontime <= 4000) // Turn Left
    // {
    //     cmd.mode = 2;
    //     cmd.gaitType = 1;
    //     //cmd.velocity[0] = 0.0f; // -1  ~ +1
    //     cmd.velocity[1] = -0.5f; // -1  ~ +1
    //     //cmd.yawSpeed = 1;
    //     cmd.footRaiseHeight = 0.3;
    // }
    // else if (motiontime > 4000 && motiontime <= 5000) // Backward
    // {
    //     cmd.mode = 2;
    //     cmd.gaitType = 1;
    //     cmd.velocity[0] = -0.5f; // -1  ~ +1
    //     cmd.yawSpeed = 0;
    //     cmd.footRaiseHeight = 0.3;
    // }
    // else if (motiontime > 5000 && motiontime <= 6000) // straightHand
    // {
    //     cmd.mode = 11;
    // }
    // else if (motiontime > 6000 && motiontime <= 7000) // dance1
    // {
    //     cmd.mode = 12;
    // }
    // else if (motiontime > 7000 && motiontime <= 8000) // dance2
    // {
    //     cmd.mode = 13;
    // }

    // motiontime += 1;
            

    //----------------------------------------------------------------------------------------------------------------------------//
    


    myfile.close();
    udp.SetSend(cmd);


    // print byte
    // use to see data send to Robodog

    //uint8_t *bytes = reinterpret_cast<uint8_t *>(&cmd);
    // for (size_t i = 0; i < sizeof(cmd); ++i)
    // {
    //     std::cout << std::hex << static_cast<int>(bytes[i]) << " ";
    // }
    // std::cout << std::endl;
}

int main(void) 
{
    std::cout << "Communication level is set to HIGH-level." << std::endl
              << "WARNING: Make sure the robot is standing on the ground." << std::endl
              << "Press Enter to continue..." << std::endl;
    std::cin.ignore();

    Custom custom(HIGHLEVEL);
    // InitEnvironment();
    LoopFunc loop_control("control_loop", custom.dt,    boost::bind(&Custom::RobotControl, &custom));
    LoopFunc loop_udpSend("udp_send",     custom.dt, 3, boost::bind(&Custom::UDPSend,      &custom));
    LoopFunc loop_udpRecv("udp_recv",     custom.dt, 3, boost::bind(&Custom::UDPRecv,      &custom));

    loop_udpSend.start();
    loop_udpRecv.start();
    loop_control.start();

    while(1){
        sleep(10);
    };

    return 0; 
}
