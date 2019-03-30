#include "mbed.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include <ros.h>
#include <std_msgs/Float32MultiArray.h>


#define TA_SHIFT 8
//the default shift for a MLX90640 device in open air


static float mlx90640To[768];
paramsMLX90640 mlx90640;

ros::NodeHandle nh;
std_msgs::Float32MultiArray msgTempratureArray1;
std_msgs::Float32MultiArray msgTempratureArray2;
std_msgs::Float32MultiArray msgTempratureArray3;
std_msgs::Float32MultiArray msgTempratureArray4;

ros::Publisher IRcam1("IRcam1", &msgTempratureArray1);
ros::Publisher IRcam2("IRcam2", &msgTempratureArray2);
ros::Publisher IRcam3("IRcam3", &msgTempratureArray3);
ros::Publisher IRcam4("IRcam4", &msgTempratureArray4);

DigitalOut led = LED1;
DigitalOut led2 = LED2;


sdsd

int main()
{
    nh.initNode();
    nh.advertise(IRcam1);
    nh.advertise(IRcam2);
    nh.advertise(IRcam3);
    nh.advertise(IRcam4);
    nh.getHardware()->setBaud(115200);
    
    MLX90640_I2CInit();
    MLX90640_I2CFreqSet(400);
    
    int status;
    uint16_t eeMLX90640[832];
    status = MLX90640_DumpEE(0x33, eeMLX90640);
      
    if (status != 0)
    {
        // pc.printf("Failed to load system parameters");
        led2 = 1;
    }
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    
    if (status != 0)
    {
        // pc.printf("Failed to load system parameters");
        led2 = 1;
    }
    
    int bufferSize = 192;
    msgTempratureArray1.data = new float[bufferSize];
    msgTempratureArray1.data_length = bufferSize;
    
    msgTempratureArray2.data = new float[bufferSize];
    msgTempratureArray2.data_length = bufferSize;
    
    msgTempratureArray3.data = new float[bufferSize];
    msgTempratureArray3.data_length = bufferSize;
    
    msgTempratureArray4.data = new float[bufferSize];
    msgTempratureArray4.data_length = bufferSize;
    
    while(1)
    {
        led = !led;
        
        for (int x = 0 ; x < 2 ; x++) //Read both subpages
        {
            uint16_t mlx90640Frame[834];
            int status = MLX90640_GetFrameData(0x33, mlx90640Frame);
            
            if (status < 0)
            {
                // pc.printf("Failed to load system parameters");
                led2 = 1;
            }
        
            float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
            float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
        
            float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
            float emissivity = 0.95;
        
            MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
        }

        
        for(int i = 0; i < bufferSize; i++)
        {
            msgTempratureArray1.data[i] = mlx90640To[i + (bufferSize * 0)];
            msgTempratureArray2.data[i] = mlx90640To[i + (bufferSize * 1)];
            msgTempratureArray3.data[i] = mlx90640To[i + (bufferSize * 2)];
            msgTempratureArray4.data[i] = mlx90640To[i + (bufferSize * 3)];
        }
        
        IRcam1.publish(&msgTempratureArray1);
        wait_ms(100);

        IRcam2.publish(&msgTempratureArray2);
        wait_ms(100);
        
        IRcam3.publish(&msgTempratureArray3);
        wait_ms(100);
        
        IRcam4.publish(&msgTempratureArray4);
        wait_ms(100);
        
        nh.spinOnce();
    }
    
    delete[] msgTempratureArray1.data;
    delete[] msgTempratureArray2.data;
    delete[] msgTempratureArray3.data;
    delete[] msgTempratureArray4.data;
    
}