
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// XDCtools Header files
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Idle.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>

#include "Board.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#define HOSTNAME          "api.openweathermap.org"
#define REQUEST_URI       "/data/2.5/forecast/?id=315202&APPID=b9bdaf75a7b1e96362a172ec83cb9303"
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define SOCKETTEST_IP     "192.168.1.33"
#define TASKSTACKSIZE     4096
#define OUTGOING_PORT     5011
#define INCOMING_PORT     5030
#define SERVERIP          "132.163.96.4"
#define SERVERPORT         37
char timeMessage[100];
int day, month, year, hour, minute , second ;
extern Semaphore_Handle semaphore0;     // posted by httpTask and pended by clientTask
extern Mailbox_Handle mailbox0;
extern Swi_Handle swi0;
extern Swi_Handle swi1;
extern Swi_Handle swi2;
extern Swi_Handle swi3;
static uint32_t trigPinFlag = 0;
char   tempstr[20];                     // temperature string
uint32_t ADCValues[2];
uint32_t rawADCValue = 0;
uint32_t count = 0;
uint32_t countTimer3 = 0;
uint32_t pinFlag = 0;
/*
 *  ======== printError ========
 */

Void timer0func(UArg arg1){
        Swi_post(swi0);
}

Void swi0func(UArg arg1, UArg arg2){
    second++;
    if(second==60){
          second=0;
          minute++;
      }
      if(minute==60){
        minute=0;
        hour++;
      }
      System_printf("%d/%d %02d:%02d:%02d", day,year,hour,minute ,second);
      System_flush();
}

bool Rec2Server(char *serverIP, int serverPort)
{
    int sockfd, connStat, numRev;
    bool retval=false;
    struct sockaddr_in serverAddr;
    char timedata[4];

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        System_printf("Socket not created");
        close(sockfd);
        return false;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));  // clear serverAddr structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);     // convert port # to network order
    inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr.s_addr));

    connStat = connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(connStat < 0) {
        System_printf("sendRec2Server::Error while connecting to server\n");
    }
    else {

        numRev = recv(sockfd,  timedata ,4,0);
        System_printf("Ntp server connected");
        unsigned long int seconds= timedata[0]*16777216 +  timedata[1]*65536 + timedata[2]*256 + timedata[3]+ 10800 ;
        System_printf("Time :%lu", seconds);
                System_flush();
        char* buf = ctime(&seconds);
                    //buf = ctime(&seconds);
                    //printf("the time is %s", buf+7);
                    //strcat(message,buf);
                    uint8_t i;


                    for(i = 0; i < 27; i++)
                    {
                        timeMessage[i] = buf[i];
                    }


                    System_printf(" the time is %s\n ", buf );
                    setTimeStr(timeMessage);

                    System_printf(" the message is: %s\n ", timeMessage);
                    System_flush();
    }

    System_flush();
    close(sockfd);
    return retval;
}

void setTimeStr(char *string)
{
    //char string[] ="Mon Jan 25 00:59:21 2021";
    int time[7];
    char *p;
    p = strtok (string," : ");
    int i=0;
    while (p!= NULL){
       time[i] = atoi(p);
       p = strtok (NULL, "  : ");
       i++;
     }
     year=time[6];

      if(time[1] == "Jan")
      {month = 01;}
      else if(time[1] == "Feb")
      {month = 02;}
      else if (time[1] == "Mar")
       {month = 03;}
      else if (time[1] == "Apr")
            {month = 04;}
      else if (time[1] == "May")
            {month = 05;}
      else if (time[1] == "Jun")
            {month = 06;}
      else if (time[1] == "Jul")
            {month = 07;}
      else if(time[1] == "Aug")
            {month = 08;}
      else if (time[1] == "Sep")
            {month = 09;}
      else if (time[1] == "Oct")
            {month = 10;}
      else if (time[1] == "Nov")
            {month = 11;}
      else if (time[1] == "Dec")
            {month = 12;}
      else
            {month = 00;}

     day=time[2];
     hour=time[3];
     minute= time[4];
     second=time[5];
     System_printf("%d/%d/%d %02d:%02d:%02d", day,month,year,hour,minute ,second);

}

void printError(char *errString, int code)
{
    System_printf("Error! code = %d, desc = %s\n", code, errString);
    BIOS_exit(code);
}

bool sendData2Server(char *serverIP, int serverPort, char *data, int size)
{
    int sockfd, connStat, numSend;
    bool retval=false;
    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        System_printf("Socket not created");
        close(sockfd);
        return false;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));  // clear serverAddr structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);     // convert port # to network order
    inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr));

    connStat = connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if(connStat < 0) {
        System_printf("sendData2Server::Error while connecting to server\n");
    }
    else {
        numSend = send(sockfd, data, size, 0);       // send data to the server
        if(numSend < 0) {
            System_printf("sendData2Server::Error while sending data to server\n");
        }
        else {
            retval = true;      // we successfully sent the temperature string
        }
    }
    System_flush();
    close(sockfd);
    return retval;
}

Void clientSocketTask(UArg arg0, UArg arg1)
{

        if(Rec2Server(SERVERIP,SERVERPORT)) {

            System_printf("clientSocketTask:: Temperature is sent to the server\n");
            System_flush();
        }
}

void getTimeStr(char *str)
{
    // dummy get time as string function
    // you may need to replace the time by getting time from NTP servers
    //
    strcpy(str, "2021-01-07 12:34:56");
}



bool createTasks(void)
{
    static Task_Handle taskHandle1;
    Task_Params taskParams;
    Error_Block eb;
    Error_init(&eb);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 1;
    taskHandle1 = Task_create((Task_FuncPtr)clientSocketTask, &taskParams, &eb);

    if (taskHandle1 == NULL) {
        printError("netIPAddrHook: Failed to create HTTP, Socket and Server Tasks\n", -1);
        return false;
    }

    return true;
}

//  This function is called when IP Addr is added or deleted
//
void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    // Create a HTTP task when the IP address is added
    if (fAdd) {
        createTasks();
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////HC-SR04

Void timer1func(UArg arg1){ //10us trigger pulse timer
         Swi_post(swi1);
}
Void swi1func(UArg arg1, UArg arg2){ //set the flag and post it with using mailbox
           trigPinFlag = 1;
           GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
           Mailbox_post(mailbox0, &trigPinFlag, BIOS_NO_WAIT);
}
Void taskCalc(UArg arg1, UArg arg2) //setup the struct for taking echo pin value
{
       while(1) {
           /* wait for event from isr (interrupt service routine! */
           Mailbox_pend(mailbox0, &pinFlag, BIOS_WAIT_FOREVER);
           //
           // Wait for conversion to be completed for sequence 3
           //
           while(!ADCIntStatus(ADC0_BASE, 3, false));
           //
           // Clear the ADC interrupt flag for sequence 3
           //
           ADCIntClear(ADC0_BASE, 3);
           //
           // Read ADC Value from sequence 3
           //
           ADCSequenceDataGet(ADC0_BASE, 3, ADCValues);
           rawADCValue = ADCValues[0];
       }
}

Void timer2func(Uarg uarg1) //this timer for the calculating waiting time, 1us period
{
    Swi_post(swi2);
}

Void swi2func(UArg arg1, UArg arg2)
{
    uint32_t countTimer3 = 0;
    while(1) {
        if((rawADCValue > 2000)&&(pinFlag==1)) { //check if adc pin value is 1 and bigger than approximately 2V
    count++; //global variable

        countTimer3++; //its the duration of echo pin
        if(countTimer3 == 38) //
        {
           break;
        }
    }
}
    else{

Swi_post(swi3);

    }
}

Void swi3func(UArg arg1, UArg arg2) // Here is the connection of two sensor
{

           distance = (count *0.0346 ) / 2; //calculate the distance value with time
            if(distance < 10){
             System_printf("distance is : %d\n", distance);
             sendData2Server(serverIP,serverPort, distance, 10);
            }
            else
            {
            System_printf("distance is : %d\n", distance);
            sendData2Server(serverIP,serverPort, distance, 10);
            }

}

void initialize_ADC()
{
    // enable ADC and Port E
    //
    SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlDelay(10);

    // Select the analog ADC function for Port E pin 3 (PE3)
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    // configure sequence 3
    //
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // every step, only PE3 will be acquired
    //
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 3);

    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////MLX90614
Void taskFxn(UArg arg0, UArg arg1)
{
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;
    i2c = I2C_open(MLX90614_ADD, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    else {
        System_printf("I2C Initialized!\n");
    }

    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 40000000);
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R3 | SYSCTL_RCGCGPIO_R9 | SYSCTL_RCGCGPIO_R1;
    result = SYSCTL_RCGCGPIO_R;
    // Enable the GPIO pin for the LED (PD3). enable the GPIO pin for digital function.
    GPIO_PORTD_AHB_DIR_R = 0x8;
    GPIO_PORTD_AHB_DEN_R = 0x8;

    GPIOPinConfigure(GPIO_PD0_I2C0SCL);
    GPIOPinConfigure(GPIO_PD1_I2C0SDA);

    GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_6);       // Configures SDA
    GPIOPinTypeI2CSCL(GPIO_PORTD_BASE,  GPIO_PIN_7);    // Configures SCL


    I2CSlaveEnable(I2C0_BASE);
    I2CSlaveInit(I2C0_BASE, MLX90614_ADD);
    I2CMasterSlaveAddrSet(I2C0_BASE, MLX90614_ADD, false);
    Swi_pend(semaphore1);
    while(1)
    {
        I2CMasterDataPut(I2C0_BASE, 0x33);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
        while(!(I2CSlaveStatus(I2C0_BASE) & I2C_SLAVE_ACT_RREQ));
        result = I2CSlaveDataGet(I2C0_BASE);
        while(!(I2CSlaveBusy(I2C0_BASE)));
         temperature = result/ 58;
            if (temperature > 36.7){
            System_printf("Abnormal degree %u: %d (C)\n", i, temperature);
            sendData2Server(serverIP,serverPort, temperature, 10);
            // Turn on the LED.
            GPIO_PORTD_AHB_DATA_R |= 0x8;
            }
            else{System_printf("Normal degree %u: %d (C)\n", i, temperature);
            sendData2Server(serverIP,serverPort, temperature, 10);}
        }
        else {
            System_printf("I2C Bus fault\n");
        }
        GPIO_PORTD_AHB_DATA_R &= ~(0x8);
    }
I2C_close(i2c);
System_printf("I2C closed!\n");

System_flush();
}
///////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();
    while(1){
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
       while(!SysCtlPeripheralReady(SYSCTL_PERIPE_GPIOE)){}
       GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_2);
       GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 1);
    }
    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the HTTP GET example\nSystem provider is set to "
            "SysMin. Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();


    /* Start BIOS */
    BIOS_start();

    return (0);
}
