#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>  
#include <math.h>
#include "stdint.h"
#define length_8 8    //定义一个宏，为传入8位16进制数的个数
#define PORT 8002
#define SERVER_IP "123.56.90.74"
#define BUFFER_SIZE 4196

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


unsigned char *fromhex(char *str)
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
	printf("启动TCP连接！\n");
	// 初始化socket dll。
	WORD winsock_version = MAKEWORD(2,2);
	//WSADATA结构包含有关Windows Sockets实现的信息。
	WSADATA wsa_data;
	//Winsock进行初始化
	//调用 WSAStartup 函数以启动使用 WS2 _32.dll
	//WSAStartup的 MAKEWORD (2，2) 参数发出对系统上 Winsock 版本2.2 的请求，并将传递的版本设置为调用方可以使用的最新版本的 Windows 套接字支持
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (iResult != 0) {
		printf("WSAStartup 失败!\n");
		WSACleanup();
		return 1;
	}
	
	// socket 函数创建绑定到特定
	//为服务器创建一个SOCKET来监听客户端连接
	//socket函数创建绑定到特定传输服务提供者的套接字。
	//参数1：地址族规范
	//参数2：新套接字的类型规范
	//参数3：使用的协议
	SOCKET client_socket = INVALID_SOCKET;
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket == INVALID_SOCKET) {
		printf("套接字错误\n");
		WSACleanup();
		return 2;
	}
	
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);
	//尝试连接到一个地址，直到一个成功	
	if (connect(client_socket, (LPSOCKADDR)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		printf("Failed to connect server: %ld !\n", GetLastError());
		closesocket(client_socket);//关闭一个已存在的套接字。
		client_socket = INVALID_SOCKET;
		return 3;
	}
	
	char recv_data[BUFFER_SIZE+1];
	while (true) {
		char *data = new char[length_8];	// 要进行输入的指令数据
		printf("具体指令给格式为0+传感器编号（1，2，3，4，5）+0300010002，请输入采集传感器的编号（0表示退出采集）：\n");
		scanf("%s",data);
		if (strcmp(data,"0")==0) {	// 输入退出
			printf("Exit!\n");
			break;
		}
		uint16_t crc;	// CRC 校验数据
		unsigned char * cmd;	// 要进行输入的控制台shuju
		char crc1[8];
		cmd = fromhex(data);	// 获取数据
		crc = CRC_16(cmd);		// 进行 CRC 校验
		uint8_t a = 0xFF;
		for(int i=0;i<6;i++){
			//TODO
			crc1[i] = cmd[i];
		}
		crc1[6] = a & crc;	// 设置最后两位为 CRC 校验位
		crc1[7] = (crc >> 8) & a;
		
		if (send(client_socket, crc1, 8, 0) < 0) {
			printf("发送失败!\n");
			break;
		}
		
		int ret = recv(client_socket, recv_data, BUFFER_SIZE, 0);
		if (ret < 0) {
			printf("接收失败!\n");
			break;
		}
		recv_data[ret]=0; // 正确结束收到的字符串
		char yb[4],wd[4];
		for(int i=0;i<4;i++){
			//TODO
			yb[i] = recv_data[4+i];
			wd[i] = recv_data[8+i];
			
		}
		float mic = hexToDec(yb)/100.0;		// 由于我们获取到的数据是十六进制，这里需要进行进制转换
		float strain_temp = hexToDec(wd)/100.0;
		printf("应变：%f\r\n",mic);
		printf("温度：%f\r\n",strain_temp);
		
		
		//printf("Receive data from server: \"%x\"\n",recv_data);
	}
	
	closesocket(client_socket);
	WSACleanup();
	
	return 0;
}



