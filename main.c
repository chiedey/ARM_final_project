/*头文件包含*/
#include "./config.h"
/*宏定义*/
#define CR      0x0000000D
#define true    0x00000001
#define LED1CON 0x00000004
#define LED2CON 0x00000008
/*全局变量定义*/
uint32  times           = 200;
uint8   normal_status[] = "No_extern_interrupts.\n";    //正常情况显示信息
uint8   exint1_status[] = "External_interrupt_No.1\n";  //外部中断1显示信息
uint8   exint2_status[] = "External_interrupt_No.2\n";  //外部中断2显示信息
/*函数声明*/
void init           (void);  //基本功能初始化函数
void target_init    (void);  //目标功能初始化函数
void interrupts_init(void);  //中断功能初始化函数
void uart0_init     (void);  //显示信息初始化函数
void send_byte      ( uint8 data );
void send_string    ( uint8 *str );
void delay          (void);  //延时2秒函数
/*中断执行函数声明*/
void __irq tc0int   (void);  //定时器0中断执行函数
void __irq exint1   (void);  //外部中断1中断执行函数
void __irq exint2   (void);  //外部中断2中断执行函数
/**
 * 主函数
 */
int main()
{
    init();
    target_init();
    send_string(normal_status);  //程序开始运行前显示无中断发生信息
    while(true);                 //程序开始运行
}
/**
 * 基本功能初始化函数
 */
void init(void)
{
    PINSEL0 = 0xa0000005;
    PINSEL1 = 0x00000000;
    IO0DIR  = LED1CON | LED2CON;
}
/**
 * 目标功能初始化函数
 */
void target_init(void)
{
    interrupts_init();
    uart0_init();
}
/**
 * 中断功能初始化函数
 */
void interrupts_init(void)
{
    T0TC         = 0x00000000;   //定时计数器初始化为0
    T0PR         = 0x00000000;   //设置定时器0预分频值
    T0MCR        = 0x00000003;   //设置T0MR0与T0TC匹配时产生中断并复位
    T0MR0        = 0x00989680;   //设置定时器0匹配值
    T0TCR        = 0x00000001;   //启动定时器0
    VICIntSelect = 0x00000000;   //选择所有中断方式为irq中断
    VICVectCntl0 = 0x00000024;   //为定时器0分配0优先级
    VICVectCntl1 = 0x0000002f;   //为外部中断1分配1优先级
    VICVectCntl2 = 0x00000030;   //为外部中断2分配2优先级
    VICVectAddr0 = (int)tc0int;  //为优先级为0的中断分配执行函数
    VICVectAddr1 = (int)exint1;	 //为优先级为1的中断分配执行函数
    VICVectAddr2 = (int)exint2;	 //为优先级为2的中断分配执行函数
    VICIntEnable = 0x00018010;   //使能定时中断器0，外部中断1、外部中断2
}
/**
 * 显示信息初始化函数
 */
void uart0_init(void)
{
    U0LCR = 0x00000083;  //8位数据，无校验，一个停止位
    U0DLL = 0x0000007a;  //波特率为9600
    U0LCR = 0x00000003;	 
}

void send_byte( uint8 data )
{
    U0THR = data;
    while( 0 == (U0LSR&0x00000020) );  
}

void send_string( uint8 *str )
{
    while(true) {
        if( '\0' == *str ) {
            send_byte('\r');
            send_byte('\n');
            break;
        }
        send_byte(*str++);
    }
}
/**
 * 延时2秒函数
 */
void delay(void)
{
    unsigned volatile long i,j;
    for( i = 0; i < 10000; i++ ) {
        for( j = 0; j < times; j++ );
    }
    times += times>200? -10:+10;
}
/**
 * 定时器0中断执行函数
 */
void __irq tc0int(void)
{
    if( 0 == (IO0PIN&(LED1CON|LED2CON)) ) {
        IO0SET = LED1CON | LED2CON;
    }
    else {
        IO0CLR = LED1CON | LED2CON;
    }
    T0IR        = 0x00000001;
    VICVectAddr = 0x00000000;
}
/**
 * 外部中断1中断执行函数
 */
void __irq exint1(void)
{
    int timeCountOfMilliSecond = 0;  //毫秒计时器
    send_string(exint1_status);	     //中断发生，显示中断信息
    times = 10;
    while( times != 200 ) {
        if( 0 == (IO0PIN&LED1CON) ) {
            IO0SET = LED1CON;
        }
        else {
            IO0CLR = LED1CON;
        }
        timeCountOfMilliSecond += (times*10);   //实时计算经过的时间
        if( 2000 <= timeCountOfMilliSecond ) {  //当经过时间等于2秒时，控制LED2灯的开关
            if( 0 == (IO0PIN&LED2CON) ) {
                IO0SET = LED2CON;
            }
            else {
                IO0CLR = LED2CON;
            }
            timeCountOfMilliSecond = 0;  //改变LED2灯状态后，毫秒计时器清零，开始下一轮计时，以实现周期性2秒闪灭效果
        }
        delay();
    }
    send_string(normal_status);  //中断结束后，显示无中断信息
    EXTINT      = 0X00000002;    //清零EINT1中断标志，为下次中断做好准备   
    VICVectAddr = 0x00000000;    //清零VICVectAddr寄存器，为下次中断做好准备
}
/**
 * 外部中断2中断执行函数
 */
void __irq exint2(void)
{
    int timeCountOfMilliSecond = 0;  //毫秒计时器
    send_string(exint2_status);	     //中断发生，显示中断信息
    times = 10;
    while( times != 200 ) {
        if( 0 == (IO0PIN&LED2CON) ) {
            IO0SET = LED2CON;
        }
        else {
            IO0CLR = LED2CON;
        }
        timeCountOfMilliSecond += (times*10);   //实时计算经过的时间
        if( 2000 <= timeCountOfMilliSecond ) {  //当经过时间等于2秒时，控制LED1灯的开关
            if( 0 == (IO0PIN&LED1CON) ) {
                IO0SET = LED1CON;
            }
            else {
                IO0CLR = LED1CON;
            }
            timeCountOfMilliSecond = 0;  //改变LED1灯状态后，毫秒计时器清零，开始下一轮计时，以实现周期性2秒闪灭效果
        }
        delay();
    }
    send_string(normal_status);  //中断结束后，显示无中断信息
    EXTINT      = 0X00000004;    //清零EINT2中断标志，为下次中断做好准备   
    VICVectAddr = 0x00000000;    //清零VICVectAddr寄存器，为下次中断做好准备
}						
