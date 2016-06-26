/* 
 * 2016.4.6일 창의과제 발표 완료. (이 파일은 오른손 arduino 용 파일)
 * -오른손데이터를 왼손에서 받아서 모바일로 보내는 형식(1:N 블루투스를 이용한다면 더 좋을듯)
 *  현재 사용중인 블루투스가 1:N이 가능하므로 향후에 개발해봐도 좋을듯?
 * -배터리는 3.7V 2개 7.4V를 이용(충전기 있음)
 * -가속도값은 LPF를 돌린값으로, 모션의 정지를 판단할때 사용.
 * -넘기는 값은 오일러각, flex sensor 값.
 * -String으로 보내서 그런지 다소 느린경향이 있음(byte 전송으로 다시 시도해봐야할듯)
 * -micro가 2kbyte ram밖에 안되서 배열을 크게 못잡는 단점(특징점을 더 찾아도 메모리때문에 넣지 못함)
 * 때문에 다른보드 사용을 권장.
 * -다른 특징점을 더 찿으면 확실히 높은 정확도를 보일것으로 예상(가속도를통한 움직임 여부 or 대략적인 벡터값)
 * goto함수 존나 많이써서 소스개몼짬.
 * bool함수 쓸데없이 너무많이씀. 지워도 될거 몇개있을듯.
 */

/*
   ex01_read_sensor.ino
   2013 withrobot Inc. (www.withrobot.com)

   myAhrs example

   Date         Author      Comment
   ----------   ----------  ---------------------------------------------------
   2013.04.26  void
*/
#include <Wire.h>

#include "Filters.h"
#include "myAhrs.h"

void gravity_remove( );
bool stop_motion();
bool acc_stop(float temp);
//void serial_flush_buffer();

myAhrs mysensor;


int16_t x, y, z;
float  x_raw, y_raw, z_raw;
int sample_count=0;

//ACC
float gravity[3];
float linear_acceleration[3];
float LPF_acc[3];


//motion
bool start_index=false, end_index=true, motion_index=true, out_index=false; 
bool x_status=false, y_status=false, z_status = false, stop_status=false, ready_status=false, end_status=false;
bool move_index = false;

//finger
uint32_t f_0, f_1, f_2, f_3, f_4; //데이터 저장.
const int flex_0 = A0, flex_1 = A1, flex_2 = A2, flex_3 = A3, flex_4 = A4;//arduino핀번호
  uint8_t  Finger_value[100][5];

//Filter(LPF)
const float alpha = 0.8;//LPF 고정값
const float filterFrequency = 10.0;//LPF 고정값
  FilterOnePole lowpassFilter_x( LOWPASS, filterFrequency ); 
  FilterOnePole lowpassFilter_y( LOWPASS, filterFrequency ); 
  FilterOnePole lowpassFilter_z( LOWPASS, filterFrequency ); 


//Angle
  int16_t Std_value[3],Last_value[3];
  int16_t  Euler_value[100][3];
  bool negative[3], postive[3];

//초기 25번 돌려주고 시작.(안정화)   
int k=25;

//Sample_count check
int check_count;
int check_index;
  
void setup()
{
  //while(!Serial);
  
  Serial.begin(115200); //시리얼 모니터용
  Serial1.begin(115200); //왼손<->오른손 통신
  
  

  if (mysensor.begin(&Wire) == false) {
    Serial.println("Initialize error");
    while (1);
  }

  Serial.print("REV ID : "); Serial.println(mysensor.revision());
  Serial.print("STATUS : ");
  if (mysensor.status() == true) {
    Serial.println("OK");
  }
  else {
    Serial.println("ERROR");                                                                       
  }
}

void loop()
{  
  
  delay(100); // 10 Hz
 // delay(10); // 100 Hz

  /*
     read sensor data from myAhrs
  */
  if (mysensor.update_data() == false) {
    Serial.println("Update error\n");
  }

  /*
     access acceleration (raw data)
     //g단위로 변환시 약 8천으로 나누면서 값이 -1~1사이값으로 측정되어 판별하기힘들다
     따라서 raw data + gravity remove 필터를 거쳐서 사용.
  */
  //초기한번만 제거.
  while(k){
      gravity_remove();
      k--;
   }
  
 
 //중력제거 필터링.
  gravity_remove( );

  lowpassFilter_x.input(linear_acceleration[0]);
  lowpassFilter_y.input(linear_acceleration[1]);
  lowpassFilter_z.input(linear_acceleration[2]);

  LPF_acc[0]=lowpassFilter_x.output();
  LPF_acc[1]=lowpassFilter_y.output();
  LPF_acc[2]=lowpassFilter_z.output();


  x_status = acc_stop(LPF_acc[0]);
  y_status = acc_stop(LPF_acc[1]);
  z_status = acc_stop(LPF_acc[2]);

   //Serial.print("Acc: ");Serial.print(LPF_acc[0],1);Serial.print("\t");Serial.print(LPF_acc[1],1);Serial.print("\t");Serial.print(LPF_acc[2],1);Serial.println("\t");
   //Serial.print("Vcc: ");Serial.print(v_x);Serial.print("\t");Serial.print(v_y);Serial.print("\t");Serial.print(v_z);Serial.println("\t");
   //Serial.print("leg: ");Serial.print(acc_length[0]);Serial.print("\t");Serial.print(acc_length[1]);Serial.print("\t");Serial.print(acc_length[2]);Serial.println("\t");Serial.println();
     
//  x = mysensor.get_raw_data(myAhrs::ATTI_ROLL);
//  y = mysensor.get_raw_data(myAhrs::ATTI_PITCH);
//  z = mysensor.get_raw_data(myAhrs::ATTI_YAW);
  /*
     access Attitude (floating point. unit(dps - degree per second))
  */
  x = mysensor.get_data(myAhrs::ATTI_ROLL);
  y = mysensor.get_data(myAhrs::ATTI_PITCH);
  z = mysensor.get_data(myAhrs::ATTI_YAW);

  stop_status = stop_motion();


  f_0 = analogRead(flex_0);
  f_1 = analogRead(flex_1);
  f_2 = analogRead(flex_2);
  f_3 = analogRead(flex_3);
  f_4 = analogRead(flex_4);



////CEHCK
//Serial.print("x : ");Serial.print(x);
//Serial.print("  y : ");Serial.print(y);
//Serial.print("  z : ");Serial.println(z);
Serial.print("  Finger: ");Serial.print(f_0);Serial.print("\t");
Serial.print(f_1);Serial.print("\t");
Serial.print(f_2);Serial.print("\t");
Serial.print(f_3);Serial.print("\t");
Serial.print(f_4);Serial.println("\t");

 
//START motion - ATT 임계값 and 가속도 0이라 가정.
    if(0){
      exit1:; //왼손으로 부터 '(' 문자 받으면
      
      if(stop_status && end_index){
       Serial.println("Start!");
       Serial1.write(')'); // ')' - transmit char to left
       start_index=true;
       ready_status=true;
       sample_count=0;
       move_index = false;
       
       Serial.println("MOTION START!!!!");
       
      //Calibration value(180이상의 변화를 알기위해 초기값을 알아야한다.)
       Std_value[0]=x;   
       Std_value[1]=y;   
       Std_value[2]=z;
       serial1_flush_buffer();  
      }
      
    }
        
   

//MOTION(recoding)
    if(start_index && ready_status){

      motion_index = true;
      end_index=false;
    Serial.println("Motion!");

//각도가 180을넘어가면 -179로 줄어들어 값으 변화가 크기때문에 181로 만들어주기위한 상태 체크
//X
      //180-> -179
      if(Last_value[0]-x >= 270){
        negative[0]=true;
        postive[0]=false;
      }
      //-180 -> 179
      else if(Last_value[0]-x <= -270){
       postive[0] = true;
       negative[0] = false;
      }

      if(Std_value[0]<=180 && Std_value[0]>=0){
        if(negative[0]){
          Euler_value[sample_count][0] = 360 + x;
        }
        else{
          Euler_value[sample_count][0] = x;
        }
      }
      else{
        if(postive[0]){
          Euler_value[sample_count][0] =  -360 + x;
        }
        else{
          Euler_value[sample_count][0] = x;
        }
      }

//Y
      //180-> -179
      if(Last_value[1] - y >= 270){
        negative[1]=true;
        postive[1]=false;
      }
      //-180 -> 179
      else if(Last_value[1] - y <= -270){
       postive[1] = true;
       negative[1] = false;
      }

      if(Std_value[1]<=180 && Std_value[1]>=0){
        if(negative[1]){
          Euler_value[sample_count][1] = 360 + y;
        }
         else{
          Euler_value[sample_count][1] = y;
        }
      }
      else{
        if(postive[1]){
          Euler_value[sample_count][1] = -360 + y;
        }
         else{
          Euler_value[sample_count][1] = y;
        }
      }
      

//Z
      //180-> -179
      if(Last_value[2]-z >= 270){
        negative[2]=true;
        postive[2]=false;
      }
      //-180 -> 179
      else if(Last_value[2]-z <= -270){
       postive[2] = true;
       negative[2] = false;
      }

      if(Std_value[2]<=180 && Std_value[2]>=0){
        if(negative[2]){
          Euler_value[sample_count][2] = z - Std_value[2] +360;
        }
        else
          Euler_value[sample_count][2] = z - Std_value[2];
      }
      else{
        if(postive[2]){
          Euler_value[sample_count][2] =  (360 - (z - Std_value[2])) * -1;
        }
        else
          Euler_value[sample_count][2] = z - Std_value[2];
      }

//메모리가 작아서 1byte로 표현하기위해 /10을 해준다.      
      Finger_value[sample_count][0] = f_0/10;
      Finger_value[sample_count][1] = f_1/10;
      Finger_value[sample_count][2] = f_2/10;
      Finger_value[sample_count][3] = f_3/10;
      Finger_value[sample_count][4] = f_4/10;

      
      sample_count++;


//OUT_move - 시작모션 이후 움직임의 여부를 판단.(각도가 벗어나던가 or 가속도의 움직임이 발생)
      if(!(move_index)){
          if(x<77 || x>97 || y<-34 || y>-7 || !(x_status) || !(y_status) || !(z_status)){
            move_index = true;
            Serial1.write('+');
            out_index = true;
            exit4:;
                out_index = true;
            }   
      }
      

        if(0){
          exit2:; // 왼손 동작이 끝났다는 메세지를 받음.
            if(out_index && stop_status && start_index){
                end_status=true;
                Serial1.write('}'); //transmit '}' char to left ( End motion)
            }
        }
            

      //OK
      if(end_status){
        Serial.println("END!");
        delay(1500);

//sample_count 맞추기.(가끔 왼손,오른손의 sample_count가 1씩 차이가난다
//이를 왼손데이터 길이만큼 맞춰준다)
              int count[4];
              check_count=0;
              check_index=1;
              sample_count=0;
              
              //Serial.print("value : ");
              char a ;
                  while(Serial1.available()){
                    a = Serial1.read();
              //Serial.print(a);Serial.print(" ");

                if(a>=48 && a<=57)
                    count[check_count++] = a;
                  
                  }
              //Serial.println();
                  for(int i=check_count-1; i>=0; i--){

                       sample_count += (count[i]-48)*check_index;

                    check_index*=10;
                  }
                  
                  //Serial.print("value : "); Serial.println(sample_count);
        
exit3:; //재전송.
       
//right_hand 출력 ("&R000000,0,0,0,0,0,0,0,0,#)
 //&R + sample_count + 3angle + 5finger + #
 
Serial1.print("&R"); //Start byte,Right byte
           if(sample_count>=100){}  //sample_count byte;
             else if(sample_count>=10){
              Serial1.print("0");
             }
             else{
              Serial1.print("00");
             }
             Serial1.print(sample_count);  Serial1.print(","); //check byte

           for(int i=0; i<sample_count; i++){
//////////////////////확인용//////////////////////////////////////
//            Euler
//              Serial.print(Euler_value[i][0]);Serial.print("\t");
//              Serial.print(Euler_value[i][1]);Serial.print("\t");
//              Serial.print(Euler_value[i][2]);Serial.print("\t");
//
//            //Finger
//               Serial.print(Finger_value[i][0]);Serial.print("\t");
//               Serial.print(Finger_value[i][1]);Serial.print("\t");
//               Serial.print(Finger_value[i][2]);Serial.print("\t");
//               Serial.print(Finger_value[i][3]);Serial.print("\t");
//               Serial.print(Finger_value[i][4]);Serial.print("\n");
/////////////////////////////////////////////////
          
            //Euler
              Serial1.print(Euler_value[i][0]);Serial1.print(",");
              Serial1.print(Euler_value[i][1]);Serial1.print(",");
              Serial1.print(Euler_value[i][2]);Serial1.print(",");

            //Finger
               Serial1.print(Finger_value[i][0]);Serial1.print(",");
               Serial1.print(Finger_value[i][1]);Serial1.print(",");
               Serial1.print(Finger_value[i][2]);Serial1.print(",");
               Serial1.print(Finger_value[i][3]);Serial1.print(",");
               Serial1.print(Finger_value[i][4]);Serial1.print(","); //End byte
//               
//
          delay(40);
        }
        Serial1.print("#");    
        serial1_flush_buffer();
            
//          }
//       }
        
        for(int i=0; i<3; i++){
          postive[i]=false;
          negative[i]=false;
        }
        
        start_index=false;
        end_index=true;
        out_index=false;
        end_status=false;
        ready_status=false;
      }
      
   }

    //Right Data receive
        while(Serial1.available()){

         char a = Serial1.read();
         //Serial.print("value : ");Serial.println(a);

          if(a=='('){  // '(' receive char from left ( motion start)
            serial1_flush_buffer();
            goto exit1;
          }
          else if(a=='{'){   // '{' receive char from left ( motion end)
            serial1_flush_buffer();
            goto exit2;
          }
          else if(a=='%'){ //re transmit
            serial1_flush_buffer();
            goto exit3;
          }
          else if(a=='+'){ // hand move check
            serial1_flush_buffer();
            goto exit4;
          }
        }
   

   Last_value[0]=x;
   Last_value[1]=y;
   Last_value[2]=z;
    
}

void gravity_remove( ){
  int remove_count=2;

  while(remove_count--){
    x_raw = mysensor.get_raw_data(myAhrs::ACCEL_X);
    y_raw = mysensor.get_raw_data(myAhrs::ACCEL_Y);
    z_raw = mysensor.get_raw_data(myAhrs::ACCEL_Z);
  
    gravity[0]=alpha * gravity[0] + (1-alpha) * x_raw;
    gravity[1]=alpha * gravity[1] + (1-alpha) * y_raw;
    gravity[2]=alpha * gravity[2] + (1-alpha) * z_raw;
  
    linear_acceleration[0] = x_raw - gravity[0];
    linear_acceleration[1] = y_raw - gravity[1];
    linear_acceleration[2] = z_raw - gravity[2];
  }

}    
bool stop_motion(){

  if(x>=77 && x<=97 && y>=-34 && y<=-7 && x_status && y_status && z_status && f_0>=400 && f_1>=400 && f_2>=400 && f_3>=400 && f_4>=400)
     return true;

  else
     return false;
}

bool acc_stop(float temp){
  float value=150;
  if(temp<=value && temp>= -value)
      return true;
  else
      return false;
}

void serial1_flush_buffer(){
  while (Serial1.read() >= 0)
   ; // do nothing
}
//void length_make(){
//
// acc_length[0] = v_x * 0.1 + acc_length[0];
// acc_length[1] = v_y * 0.1 + acc_length[1];
// acc_length[2] = v_z * 0.1 + acc_length[2];
//  
//  }
//
//void velocity(){
//
//  v_x = LPF_acc[0]*0.1 + v_x;
//  v_y = LPF_acc[1]*0.1 + v_y;
//  v_z = LPF_acc[2]*0.1 + v_z;
//  
//  }
//
//void print_3d_int(const char* tag_x, const char* tag_y, const char* tag_z, int x, int y, int z)
//{
//  Serial.print(tag_x); Serial.print("("); Serial.print(x); Serial.print("), ");
//  Serial.print(tag_y); Serial.print("("); Serial.print(y); Serial.print("), ");
//  Serial.print(tag_z); Serial.print("("); Serial.print(z); Serial.print("), ");
//}
//
//void print_1d_int(const char* tag_x, int x)
//{
//  Serial.print(tag_x); Serial.print("("); Serial.print(x); Serial.print("), ");
//}
//
//void print_3d_float(const char* tag_x, const char* tag_y, const char* tag_z, float x, float y, float z)
//{
//  Serial.print(tag_x); Serial.print("("); Serial.print(x, 4); Serial.print("), ");
//  Serial.print(tag_y); Serial.print("("); Serial.print(y, 4); Serial.print("), ");
//  Serial.print(tag_z); Serial.print("("); Serial.print(z, 4); Serial.print("), ");
//}
//
//void print_1d_float(const char* tag_x, float x)
//{
//  Serial.print(tag_x); Serial.print("("); Serial.print(x, 4); Serial.print("), ");
//}
