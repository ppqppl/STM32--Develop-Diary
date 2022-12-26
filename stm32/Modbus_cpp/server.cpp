#include <winsock2.h>	//传输通信
#include <ws2tcpip.h>	//用于检索ip地址的新函数和结构
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")//引入ws2_32.lib库，不然编译报错
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define DEFAULT_IP "127.0.0.1"// 服务器为本机
#define DEFAULT_PORT "27015" //默认端口
#define DEFAULT_BUFLEN 512 	//字符缓冲区长度


int main() {
	
	printf("启动服务器!\n");
	
	#pragma region 1. 初始化
	
	WSADATA wsaData;	// 定义一个结构体成员，存放的是 Windows Socket 初始化信息
	//Winsock进行初始化
	//调用 WSAStartup 函数以启动使用 WS2 _32.dll
	int iResult;		// 函数返回数据
	//WSAStartup的 MAKEWORD (2，2) 参数发出对系统上 Winsock 版本2.2 的请求，并将传递的版本设置为调用方可以使用的最版本的 Windows 套接字支持
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);	// 启动命令，如果返回为 0 ，说明成功启动
	
	if (iResult != 0) {	// 返回不为 0 启动失败
		printf("初始化Winsock出错: %d\n", iResult);
		return 1;
	}
	
	#pragma endregion 1. 初始化结束
	
	
	#pragma region 2. 为服务器创建套接字
	
	struct addrinfo* result = NULL,	* ptr = NULL, hints;
	
	ZeroMemory(&hints, sizeof(hints));	// 将内存块的内容初始化为零
	hints.ai_family = AF_INET; 			//AF _INET 用于指定 IPv4 地址族
	hints.ai_socktype = SOCK_STREAM;	// SOCK _STREAM 用于指定流套接字
	hints.ai_protocol = IPPROTO_TCP;	// IPPROTO _TCP 用于指定 tcp 协议
	hints.ai_flags = AI_PASSIVE;		// 指定 getaddrinfo 函数中使用的选项的标志。AI_PASSIVE表示：套接字地址将在调用 bindfunction 时使用
	
	// 从本机中获取 ip 地址等信息为了 sockcet 使用
	//getaddrinfo 函数提供从 ANSI 主机名到地址的独立于协议的转换。
	//参数1：该字符串包含一个主机(节点)名称或一个数字主机地址字符串。
	//参数2：服务名或端口号。
	// 参数3：指向 addrinfo 结构的指针，该结构提供有关调用方支持的套接字类型的提示。
	//参数4：指向一个或多个包含主机响应信息的 addrinfo 结构链表的指针。
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("解析地址/端口失败: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	
	// 创建socket对象，使服务器侦听客户端连接
	SOCKET ListenSocket = INVALID_SOCKET;
	// socket 函数创建绑定到特定
	//为服务器创建一个SOCKET来监听客户端连接
	//socket函数创建绑定到特定传输服务提供者的套接字。
	//参数1：地址族规范
	//参数2：新套接字的类型规范
	//参数3：使用的协议
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {	//检查是否有错误，以确保套接字为有效的套接字
		printf("套接字错误: %ld\n", WSAGetLastError());
		freeaddrinfo(result);	 //调用 freeaddrinfo 函数以释放由 getaddrinfo 函数为此地址信息分配的内存。
		WSACleanup();			//终止 WS2 _ 32 DLL 的使用
		return 1;
	}
	
	#pragma endregion 2. 创建套接字结束
	
	#pragma region 3. 绑定套接字
	
	//要使服务器接受客户端连接，必须将其绑定到系统中的网络地址。
	//Sockaddr结构保存有关地址族、IP 地址和端口号的信息。
	//bind函数将本地地址与套接字关联起来。设置TCP监听套接字
	//参数1：标识未绑定套接字的描述符。
	//2：一个指向本地地址sockaddr结构的指针，用于分配给绑定的套接字。这里面有Sockaddr结构
	//3：所指向值的长度(以字节为单位)
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("设置TCP监听套接字失败: %d\n", WSAGetLastError());
		freeaddrinfo(result);		// 调用 bind 函数后，不再需要地址信息 释放
		closesocket(ListenSocket);	// 关闭一个已存在的套接字
		WSACleanup();
		return 1;
	}
	
	#pragma endregion 3. 绑定套接字结束
	
	#pragma region 4. 在套接字上监听客户端(监听套接字)
	
	//将套接字绑定到系统的ip地址和端口后，服务器必须在IP地址和端口上监听传入的连接请求
	//listen函数将套接字置于侦听传入连接的状态。
	//参数1：标识已绑定的未连接套接字的描述符。
	//2：挂起连接队列的最大长度。如果设置为SOMAXCONN，负责套接字的底层服务提供者将把待办事项设置为最大合理值
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		// SOMAXCONN定义了此套接字允许最大连接
		printf("监听传入失败: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);	// 关闭一个已连接的套接字
		WSACleanup();
		return 1;
	}
	
	#pragma endregion 4. 在套接字上监听客户端(监听套接字)结束
	
	#pragma region 5.接受来自客户端的连接（Windows 插槽 2）
	
	//当套接字监听连接后，程序必须处理套接字上的连接请求
	//创建临时套接字对象，以接受来自客户端的连接
	SOCKET ClientSocket;
	
	//通常，服务器应用程序将被设计为侦听来自多个客户端的连接。 对于高性能服务器，通常使用多个线程来处理多个客户端连接。 这个示例比较简单，不用多线程
	
	ClientSocket = INVALID_SOCKET; //INVALID_SOCKET定义代表遮套接字无效
	//accept函数允许套接字上的传入连接尝试
	//参数1：一个描述符，用来标识一个套接字，该套接字使用listen函数处于侦听状态。连接实际上是用accept返回的套接字建立的。
	//2：一种可选的指向缓冲区的指针，用于接收通信层所知的连接实体的地址。addr参数的确切格式是由当socket来自so时建立的地址族决定的
	//3:一个可选的指针，指向一个整数，该整数包含addr参数所指向的结构的长度。
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("传入连接失败: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	/*注意：当客户端连接被接受后，服务器应用程序通常会将接受的客户端套接字传递 (ClientSocket 变量) 到工作线程或 i/o 完成端口，并继续接受其他连接。
	这个示例没有，可以查看Microsoft Windows 软件开发工具包 (SDK) 附带的 高级 Winsock 示例 中介绍了其中部分编程技术的示例。 
	链接：https://docs.microsoft.com/zh-cn/windows/win32/winsock/getting-started-with-winsock*/
	#pragma endregion 5.接受来自客户端的连接(Windows 插槽 2)结束
	
	#pragma region 6. 在服务器上接收和发送数据
	
	char recvbuf[DEFAULT_BUFLEN]; 		//字符缓冲区数组
	int  iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;	//缓冲值
	
	do {
		//recv函数从已连接的套接字或已绑定的无连接套接字接收数据。
		//参数1：套接字描述符
		//参数2：一个指向缓冲区的指针，用来接收传入的数据。
		//参数3：参数buf所指向的缓冲区的长度，以字节为单位。
		//参数4：一组影响此函数行为的标志
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("接收的字节数: %d\n", iResult);
			//将缓冲区回传给发送方
			//发送一个初始缓冲区
			//send函数参数1：标识已连接套接字的描述符。
			//参数2：指向包含要传送的数据的缓冲区的指针。这里为了简单将客户端发送过来的消息再发送给客户端 
			//参数3：参数buf所指向的缓冲区中数据的长度(以字节为单位)。strlen获取字符串长度
			//参数4：指定调用方式的一组标志。
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("发送失败: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("字节发送: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("连接关闭...\n");
		else {
			printf("接受失败: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);
	
	#pragma endregion 6. 在服务器上接收和发送数据结束
	
	#pragma region 7. 断开服务器连接
	
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("关闭失败: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	
	/*第二种关闭方法
	使用 Windows 套接字 DLL 完成客户端应用程序时，将调用 WSACleanup 函数来释放资源。
	closesocket(ClientSocket);
	WSACleanup();*/
	
	#pragma endregion 7. 断开服务器连接结束
	
	return 0;
}
