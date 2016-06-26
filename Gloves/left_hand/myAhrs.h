/*
 * myAhrs.h
 * 2013 withrobot Inc. (www.withrobot.com)
 *
 * myAhrs driver
 *
 * Date         Author      Comment
 * ----------   ----------  ---------------------------------------------------
 * 2013.08.23   void        
 */


 #ifndef __MY_ACCEL_GYRO_H__
 #define __MY_ACCEL_GYRO_H__
//MY_ACCEL_GYRO_H__가 선언되어 있지 않으면 컴파일
//=>즉 중복 컴파일 방지.

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdint.h>

 #include <Arduino.h>
 #include <Wire.h>

 class myAhrs {
     public:
         enum Did{
             ACCEL_X = 0,
             ACCEL_Y,
             ACCEL_Z,
             GYRO_X,
               GYRO_Y,  
             GYRO_Z,
             MAG_X,
             MAG_Y,
             MAG_Z, 
             ATTI_ROLL,
             ATTI_PITCH,
             ATTI_YAW,
             TEMP,
         };
 
     private : 
         #pragma pack(1)  //1바이트 단위로 저장.
         struct {
           uint16_t ax;
           uint16_t ay;
           uint16_t az;
           
           uint16_t gx;
           uint16_t gy;
           uint16_t gz;
           
           uint16_t mx;
           uint16_t my;
           uint16_t mz;
           
           uint16_t roll;
           uint16_t pitch;
           uint16_t yaw;           
                      
           uint16_t temp;
         } raw_data;     

         double acc_scale_factor, gyro_scale_factor, magnet_scale_factor, attitude_scale_factor, temp_scale_factor;  
         TwoWire* two_wire;
         bool sensor_init;

         bool write(uint8_t reg_add, uint8_t * buff, uint8_t len);
         bool read(uint8_t reg_add, uint8_t * buff, uint8_t len);

     public:
         myAhrs();

         /*
          * Initialize myAhrs
          */        
         bool begin(TwoWire * two_wire=0);

         /*
          * Read info.
          */
         int  revision();
         bool status();

         /*
          * Control 
          */
         bool ctrl_reset();
         bool ctrl_save_changes();
         bool ctrl_restore_factory_setting();

         /*
          * Change settings  
          */    
     private: 
         bool update_option(uint8_t reg, uint8_t opt);
         
     public: 
         /*
          * read sensor values from myAhrs
          */
         bool update_data();

         /*
          * access sensor values 
          */        
         int   get_raw_data(Did id);
         float get_data(Did id);     
 };

//typedef struct data{
//  uint8_t  Finger_value[100][5];
//  int16_t  Euler_value[100][3];
//}data_hand_t;
//
//typedef struct packet{
//  
//  char start_byte;
//  char second_byte;
//  char direct_byte;
//  char size_byte;
//  char command_byte;
//  data_hand_t data;
//  char crc_byte;
//
//}packet_hand_t;


  

 #endif /* __MY_ACCEL_GYRO_H__ */

