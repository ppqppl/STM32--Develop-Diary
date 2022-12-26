#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>  
#include <math.h>
#include "stdint.h"
#define length_8 8    //定义一个宏，为传入8位16进制数的个数
#define PORT 8001
#define SERVER_IP "123.56.90.74"
#define  BUFFER_SIZE 4196

const char* kExitFlag = "exit";

/* 返回ch字符在sign数组中的序号 */
int getIndexOfSigns(char ch)
{
	if(ch >= '0' && ch <= '9')
	{
		return ch - '0';
	}
	if(ch >= 'A' && ch <='F') 
	{
		return ch - 'A' + 10;
	}
	if(ch >= 'a' && ch <= 'f')
	{
		return ch - 'a' + 10;
	}
	return -1;
}
/* 十六进制数转换为十进制数 */
int hexToDec(char *source)
{
	int sum = 0;
	char low,high;
	for(int i=0,j=7;i<4;i++){
		//TODO
		high = (source[i] & 0xf0)>>4;
		low = source[i] & 0x0f;
		sum += high*pow(16,j--)+low*pow(16,j--); 
	}
	return sum;
}



unsigned char *fromhex(const char *str)
{
	static unsigned char buf[512];
	size_t len = strlen(str) / 2;
	if (len > 512) len = 512;
	for (size_t i = 0; i < len; i++) {
		unsigned char c = 0;
		if (str[i * 2] >= '0' && str[i*2] <= '9') 
			c += (str[i * 2] - '0') << 4;
		if ((str[i * 2] & ~0x20) >= 'A' && (str[i*2] & ~0x20) <= 'F') 
			c += (10 + (str[i * 2] & ~0x20) - 'A') << 4;
		if (str[i * 2 + 1] >= '0' && str[i * 2 + 1] <= '9') 
			c += (str[i * 2 + 1] - '0');
		if ((str[i * 2 + 1] & ~0x20) >= 'A' && (str[i * 2 + 1] & ~0x20) <= 'F')
			c += (10 + (str[i * 2 + 1] & ~0x20) - 'A');
		buf[i] = c;
	}
	return buf;
}

uint16_t CRC_16(uint8_t *temp)
{
	uint8_t i,j;
	uint16_t CRC_1 = 0xFFFF;          //声明CRC寄存区，也就是步骤1
	for(i = 0;i < 6;i++)       //这里的for循环说的是步骤6中的重复步骤 2 到步骤 5
	{
		CRC_1 ^= temp[i]; //这里就是步骤2，进行异或运算
		for(j = 0;j < 8;j++)         //用来将异或后的低八位全部移出的for循环
		{
			if(CRC_1 & 0x01)         //判断低八位的最后一位是否为1，为1时执行下列语句，也就是步骤3说的移位判断与步骤5说的右移8次
			{
				/*一定要先移位，再异或*/
				CRC_1 >>=1;          //移位后再异或，就是步骤4
				CRC_1 ^= 0xA001;     //0xA001为0x8005的逆序
			}
			else                    //若不为1，则直接移位。
			{
				CRC_1 >>=1;
			}
		}
	}
	
	//	CRC_1 = (((CRC_1 & 0xFF)<<8) + (CRC_1>>8));
	//	printf("%04x\r\n",CRC_1);     //用于打印检测CRC校验码
	return(CRC_1);
}

int main() {
	// 初始化socket dll。
	WORD winsock_version = MAKEWORD(2,2);
	WSADATA wsa_data;
	if (WSAStartup(winsock_version, &wsa_data) != 0) {
		printf("Failed to init socket!\n");
		return 1;
	}
	
	SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_socket == INVALID_SOCKET) {
		printf("Failed to create server socket!\n");
		return 2;
	}
	
	char recv_data[BUFFER_SIZE+1];
	
	while (true) {
//		uint8_t data[length_8];
		char *data = new char[length_8];
		printf("具体指令给格式为0+传感器编号（1，2，3，4，5）0300010002");
		printf("请输入采集传感器的指令） 输入exit退出：\r\n");
		scanf("%s",data);
		if (strcmp(data,kExitFlag)==0) {
			printf("Exit!\n");
			break;
		}
		uint16_t crc;
		unsigned char * cmd;
		char crc1[8];
		cmd = fromhex(data);
		crc = CRC_16(cmd);
		uint8_t a = 0xFF;
		for(int i=0;i<6;i++){
			//TODO
			crc1[i] = cmd[i];
		}
		crc1[6] = a & crc;
		crc1[7] = (crc >> 8) & a;
		
		struct sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(PORT);
		server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
		
		if (sendto(client_socket, crc1, 8, 0,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0) {
			printf("Failed to send data!\n");
			break;
		}
		
		int ret = recvfrom(client_socket, recv_data, BUFFER_SIZE, 0,NULL,NULL);
		if (ret < 0) {
			printf("Failed to receive data!\n");
			break;
		}
		recv_data[ret]=0; // correctly ends received string
		char var = cmd[5];
		if(var == 2){
			printf("两个传感器：温度，湿度\r\n");
			//TODO
			char yb[4],wd[4];
			for(int i=0;i<4;i++){
				//TODO
				yb[i] = recv_data[4+i];
				wd[i] = recv_data[8+i];
				
			}
			float temp = hexToDec(yb)/100.0;
			float hum = hexToDec(wd);
			printf("温度：%4.2f\r\n",temp);
			printf("湿度：%4.2f\r\n",hum);
		}else if(var == 1){
			//TODO
			printf("一个传感器：静力水准仪\r\n");
			char nd[4];
			for(int i=0;i<4;i++){
				//TODO
				nd[i] = recv_data[4+i];
			}
			float water_level = hexToDec(nd)*10.0;
			printf("挠度：%6.2f\r\n",water_level);
		}
	}
	
	closesocket(client_socket);
	WSACleanup();
	
	return 0;
}

