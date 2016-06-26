/*
 * myAhrs.cpp
 * 2013 withrobot Inc. (www.withrobot.com)
 *
 * myAhrs driver
 *
 * Date         Author      Comment
 * ----------   ----------  ---------------------------------------------------
 * 2013.08.23   void        
 */

 #include "myAhrs.h"

 #define MYAHRS_I2C_ADDRESS           0x20//check
 #define MYMOTION_WHO_AM_I_VALUE      0xB1//check

 /*
  * REGISTER MAP 
  */
 #define REGISTER_WHO_AM_I            0x01//check
 #define REGISTER_REV_ID              0x02//check(major)
 #define REGISTER_STATUS              0x04//check
     #define STAT_READY                  0x80//check

 #define REGISTER_CONTROL             0x10
     #define CTRL_RESET                  0x80
     #define CTRL_RESTORE_FACTORY        0x40
     #define CTRL_SAVE_NV                0x10

 #define REGISTER_SENSOR_ID           0x12//check
 #define REGISTER_OUTPUT_RATE         0x13//check

 // ACC Control Register
 #define REGISTER_ACC_RANGE           0x21//check
 #define REGISTER_ACC_FILTER          0x22//check

 // GYRO Control Register
 #define REGISTER_GYRO_RANGE          0x2A//check
 #define REGISTER_GYRO_FILTER         0x2B//check

 // ACC Data Register
 #define REGISTER_ACC_X_L             0x22//check 
 #define REGISTER_ACC_X_H             0x23//check
 #define REGISTER_ACC_Y_L             0x24//check
 #define REGISTER_ACC_Y_H             0x25//check
 #define REGISTER_ACC_Z_L             0x26//check
 #define REGISTER_ACC_Z_H             0x27//check

 // GYRO Data Register
 #define REGISTER_GYRO_X_L            0x28//check
 #define REGISTER_GYRO_X_H            0x29//check
 #define REGISTER_GYRO_Y_L            0x2A//check
 #define REGISTER_GYRO_Y_H            0x2B//check
 #define REGISTER_GYRO_Z_L            0x2C//check
 #define REGISTER_GYRO_Z_H            0x2D//check
 
// Magnetometer 
#define REGISTER_MAGNET_X_L           0x2E//check
#define REGISTER_MAGNET_X_H           0x2F//check
#define REGISTER_MAGNET_Y_L           0x30//check
#define REGISTER_MAGNET_Y_H           0x31//check
#define REGISTER_MAGNET_Z_L           0x32//check
#define REGISTER_MAGNET_Z_H           0x33//check
                                        
// Attitude                           
#define REGISTER_ATTI_ROLL_L          0x36//check
#define REGISTER_ATTI_ROLL_H          0x37//check
#define REGISTER_ATTI_PITCH_L         0x38//check
#define REGISTER_ATTI_PITCH_H         0x39//check
#define REGISTER_ATTI_YAW_L           0x3A//check
#define REGISTER_ATTI_YAW_H           0x3B//check

 // TEMPERATURE Data Register
 #define REGISTER_TEMP_L              0x34//check
 #define REGISTER_TEMP_H              0x35//check

 myAhrs::myAhrs() 
     : two_wire(0)
     , acc_scale_factor(16.0/32767.0)      
     , gyro_scale_factor(2000.0/32767.0)   //check
     , magnet_scale_factor(0.3)            //check
     , attitude_scale_factor(180.0/32767.0)//check
     , temp_scale_factor(200.0/32767.0)    
     , sensor_init(false)
 {
     memset(&raw_data, 0, sizeof(raw_data));
 }

 bool myAhrs::begin(TwoWire * tw)
 {
     int retry = 3;
     two_wire = tw;

     if(!two_wire) {
         return false; 
     }

     two_wire->begin();

     uint8_t id, stat;

     if(read(REGISTER_WHO_AM_I, &id, 1) == false) {
         return false;
     }

     while(retry-- > 0 && sensor_init == false) {
         if(read(REGISTER_STATUS, &stat, 1) == false) {
             return false;
         }    

         if(id == MYMOTION_WHO_AM_I_VALUE && stat == STAT_READY) {
             sensor_init = true;
         }
     }

     return sensor_init;
 }

 bool myAhrs::write(uint8_t reg_add, uint8_t* buff , uint8_t len)
 {
     if(!two_wire) {
         return false;
     }

     two_wire->beginTransmission((uint8_t)MYAHRS_I2C_ADDRESS); 

     two_wire->write(reg_add);                          

     for(uint8_t cnt=0; cnt<len; cnt++) {
         two_wire->write(buff[cnt]);                   
     }

     return two_wire->endTransmission(true) == 0;   
 }

 bool myAhrs::read(uint8_t reg_add, uint8_t* buff , uint8_t len)
 {
     if(!two_wire) {
         return false;
     }

     two_wire->beginTransmission((uint8_t)MYAHRS_I2C_ADDRESS);  

     two_wire->write(reg_add); 

     two_wire->endTransmission(false); 

     two_wire->requestFrom((uint8_t)MYAHRS_I2C_ADDRESS, len);

     uint8_t cnt = 0;
     while(two_wire->available()) {
         buff[cnt++] = two_wire->read();
     }

     return (cnt == len);
 }

 int myAhrs::revision() 
 {
     uint8_t rev;

     if(sensor_init == false) { 
         return -1;
     }

     if(read(REGISTER_REV_ID, &rev, 1) == false) {
         return -1;
     }

     return rev;            
 }

 bool myAhrs::status() 
 {
     uint8_t stat;

     if(sensor_init == true) {
         if(read(REGISTER_STATUS, &stat, 1) == false) {
             sensor_init = false;
         }
         sensor_init = (stat == STAT_READY);
     }

     return sensor_init;
 }

 bool myAhrs::ctrl_reset() 
 {
     uint8_t ctrl = CTRL_RESET;
     return write(REGISTER_CONTROL, &ctrl, 1);
 }

 bool myAhrs::ctrl_save_changes() 
 {
     uint8_t ctrl = CTRL_SAVE_NV;
     return write(REGISTER_CONTROL, &ctrl, 1);            
 }

 bool myAhrs::ctrl_restore_factory_setting() 
 {
     uint8_t ctrl = CTRL_RESTORE_FACTORY;
     return write(REGISTER_CONTROL, &ctrl, 1);            
 }

 bool myAhrs::update_option(uint8_t reg, uint8_t opt) 
 {
     uint8_t ropt;

     if(sensor_init == false) { 
         return false;
     }    

     if(write(reg, &opt, 1) == false) {
         return false;
     }

     delay(100);

     if(read(reg, &ropt, 1) == false) {
         return false;
     }        

     return (opt == ropt);    
 }   

 bool myAhrs::update_data() 
 {
     if(sensor_init == false) { 
         return false;
     }

     // read ACC, GYRO, MAGNETOMETER
     if(read(REGISTER_ACC_X_L, (uint8_t*)&raw_data, sizeof(uint16_t)*9) == false) {
         return false;
     }

     // read ATTITUDE 
     if(read(REGISTER_ATTI_ROLL_L, (uint8_t*)&raw_data.roll, sizeof(uint16_t)*3) == false) {
         return false;
     }
     
     // read TEMPERATURE
     return read(REGISTER_TEMP_L, (uint8_t*)&raw_data.temp, sizeof(uint16_t));
 }

 /*
  * accessor
  */        
 int myAhrs::get_raw_data(Did id) 
 {
     switch(id) {
         case ACCEL_X: return raw_data.ax;
         case ACCEL_Y: return raw_data.ay;
         case ACCEL_Z: return raw_data.az;
         
         case GYRO_X:  return raw_data.gx;                
         case GYRO_Y:  return raw_data.gy;
         case GYRO_Z:  return raw_data.gz;  

         case MAG_X:  return raw_data.mx;                
         case MAG_Y:  return raw_data.my;
         case MAG_Z:  return raw_data.mz;

         case ATTI_ROLL :  return raw_data.roll;                
         case ATTI_PITCH:  return raw_data.pitch;
         case ATTI_YAW  :  return raw_data.yaw;         
         
         case TEMP:    return raw_data.temp;  
         default: return 0;                                              
     }
 } 

 float myAhrs::get_data(Did id) 
 {
     double sf;

     switch(id) {        
         case ACCEL_X: 
         case ACCEL_Y: 
         case ACCEL_Z: 
             sf = acc_scale_factor;
             break; 
             
         case GYRO_X:                
         case GYRO_Y:
         case GYRO_Z:  
             sf = gyro_scale_factor; 
             break;

         case MAG_X:                
         case MAG_Y:
         case MAG_Z:  
             sf = magnet_scale_factor; 
             break;
         
         case ATTI_ROLL :                
         case ATTI_PITCH:
         case ATTI_YAW  :  
             sf = attitude_scale_factor; 
             break;
              
         case TEMP:
             sf = temp_scale_factor;
             break;
             
         default: 
             return 0;                                              
     }

     return (float)(sf * get_raw_data(id)); 
 }
