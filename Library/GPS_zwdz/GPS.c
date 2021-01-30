

#include "stdio.h"
#include "string.h"
#include "GPS.h"

gps_msg gpsmsg;
E53_ST1_Data_TypeDef E53_ST1_Data;

E53_ST1_Data_TypeDef getData(){
    return E53_ST1_Data;
}
/***************************************************\
* 函数名称: NMEA_Comma_Pos
*	函数功能：从buf里面得到第cx个逗号所在的位置
*	输入值：
*	返回值：0~0xFE，代表逗号所在位置的便宜
*				 	0xFF，代表不存在第cx个逗号
\***************************************************/

uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{
    uint8_t *p = buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')return 0xFF;
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;
}
/***************************************************\
* 函数名称: NMEA_Pow
*	函数功能：返回m的n次方值
*	输入值：底数m和指数n
*	返回值：m^n
\***************************************************/
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
    uint32_t result = 1;
    while(n--)result *= m;
    return result;
}
/***************************************************\
* 函数名称: NMEA_Str2num
*	函数功能：str数字转换为（int）数字，以','或者'*'结束
*	输入值：buf，数字存储区
*				 	dx，小数点位数，返回给调用函数
*	返回值：转换后的数值
\***************************************************/
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
    uint8_t *p = buf;
    uint32_t ires = 0,fres = 0;
    uint8_t ilen = 0,flen = 0,i;
    uint8_t mask = 0;
    int res;
    while(1)
    {
        if(*p=='-'){mask |= 0x02;p++;}//说明有负数
        if(*p==','||*p=='*')break;//遇到结束符
        if(*p=='.'){mask |= 0x01;p++;}//遇到小数点
        else if(*p>'9'||(*p<'0'))//数字不在0和9之内，说明有非法字符
        {
            ilen = 0;
            flen = 0;
            break;
        }
        if(mask&0x01)flen++;//小数点的位数
        else ilen++;//str长度加一
        p++;//下一个字符
    }
    if(mask&0x02)buf++;//移到下一位，除去负号
    for(i=0;i<ilen;i++)//得到整数部分数据
    {
        ires += NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;//最多取五位小数
    *dx = flen;
    for(i=0;i<flen;i++)//得到小数部分数据
    {
        fres +=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    }
    res = ires*NMEA_Pow(10,flen)+fres;
    if(mask&0x02)res = -res;
    return res;
}
/***************************************************\
* 函数名称: NMEA_BDS_GPRMC_Analysis
*	函数功能：解析GNRMC信息
*	输入值：gpsx,NMEA信息结构体
*				 buf：接收到的GPS数据缓冲区首地址
\***************************************************/
void NMEA_BDS_GPRMC_Analysis(gps_msg *gpsmsg,uint8_t *buf)
{
    uint8_t *p4,dx;
    uint8_t posx;
    uint32_t temp;
    float rs;
    p4=(uint8_t*)strstr((const char *)buf,"$GNRMC");//"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
    posx=NMEA_Comma_Pos(p4,3);								//得到纬度
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p4+posx,&dx);
        gpsmsg->latitude_bd=temp/NMEA_Pow(10,dx+2);	//得到°
        rs=temp%NMEA_Pow(10,dx+2);				//得到'
        gpsmsg->latitude_bd=gpsmsg->latitude_bd*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为°
    }
    posx=NMEA_Comma_Pos(p4,4);								//南纬还是北纬
    if(posx!=0XFF)gpsmsg->nshemi_bd=*(p4+posx);
    posx=NMEA_Comma_Pos(p4,5);								//得到经度
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p4+posx,&dx);
        gpsmsg->longitude_bd=temp/NMEA_Pow(10,dx+2);	//得到°
        rs=temp%NMEA_Pow(10,dx+2);				//得到'
        gpsmsg->longitude_bd=gpsmsg->longitude_bd*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为°
    }
    posx=NMEA_Comma_Pos(p4,6);								//东经还是西经
    if(posx!=0XFF)gpsmsg->ewhemi_bd=*(p4+posx);
}
static unsigned char gps_uart[1000];
void E53_ST1_Read_Data(void)
{
    /* 使用中断方式接收一次数据 */
    HAL_UART_Receive(&huart3,gps_uart,1000,1000);
    /* 分析缓冲区的字符串，解析GPS数据 */
    NMEA_BDS_GPRMC_Analysis(&gpsmsg,(uint8_t*)gps_uart);

    /* 将解析到的经纬度数据存放到结构体中，便于其他函数使用 */
    E53_ST1_Data.Longitude=(float)((float)gpsmsg.longitude_bd/100000);
    E53_ST1_Data.Latitude=(float)((float)gpsmsg.latitude_bd/100000);
}

//初始化GPS
void GPSinit(){
    uint8_t NMEA_Comma_Pos(uint8_t *buf, uint8_t cx);

    uint32_t NMEA_Pow(uint8_t m, uint8_t n);

    int NMEA_Str2num(uint8_t *buf, uint8_t *dx);

    void NMEA_BDS_GPRMC_Analysis(gps_msg *gpsmsg, uint8_t *buf);

    void E53_ST1_Read_Data(void);
}




