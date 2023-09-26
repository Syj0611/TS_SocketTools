/*
 * @Author: seven 865762826@qq.com
 * @Date: 2023-08-04 10:25:26
 * @LastEditors: seven 865762826@qq.com
 * @LastEditTime: 2023-08-22 09:52:22
 * @FilePath: \window_linux_Rep\c++\TSFlexray\include\TSSocket.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H
#include<iostream>
#if defined(__linux__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h> 
bool init_tcp_socket(int& server_fd,int& opt) {
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        return false;
    }
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        return false;
    }
    return true;
}

bool init_udp_socket(int& server_fd,int& opt){
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        return false;
    }
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        return false;
    }
    return true;
}
void close_socket(int server_fd) {
    close(server_fd);
}


bool bind_socket(int server_fd, const char*ip,int port) {
    // Forcefully attaching socket to the port 8080
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr))<0) {
        return false;
    }
    return true;
}

bool listen_socket(int server_fd,int backlog) {
    if (listen(server_fd, backlog) < 0) {
        return false;
    }
    return true;
}

bool accept_socket(int server_fd, struct sockaddr_in& address, int& new_socket) {
    int addrlen = sizeof(sockaddr_in);
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        return false;
    }
    struct timeval timeout;
    timeout.tv_sec = 5; // Set timeout to 5 seconds
    timeout.tv_usec = 0;
    return true;
}
void connect_socket(int server_fd, struct sockaddr_in& address) {
    if (connect(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }
}

bool udp_send_socket(int server_fd,const char* hello,int buffer_size, sockaddr* address)
{
    int sent_bytes = sendto(server_fd,hello,buffer_size,0,address,sizeof(sockaddr_in));
    if(sent_bytes == -1)
        return false;
    return true;
}

bool send_socket(int new_socket, const char* hello,int buffer_size) {
    try{
    int sent_bytes = send(new_socket, hello, buffer_size, 0);
    if (sent_bytes == -1) {
        return false;
    }
    }
    catch(const std::exception& e)
    {
        return false;
    }
    return true;
}

int recv_udp_socket(int server_fd,char*buffer,s32 len,sockaddr* address,socklen_t*addrlen)
{
    return recvfrom(server_fd, buffer, len, 0, address,addrlen);

}

void recv_socket(int new_socket, char* buffer, int* buffer_size) {
    int valread = read(new_socket, buffer, *buffer_size);
    *buffer_size = valread;
}
#endif

#if defined(_WIN32)
#include<WinSock2.h>	//头文件
#pragma comment(lib,"ws2_32.lib")
bool init_socket()
{
    //初始化代码
	WORD wVersion = MAKEWORD(2, 2);
	//MAKEWORD：将两个byte型合成一个word型，一个在高八位，一个在低八位
	//MAKEWORD(1,1)只能一次接收一次，不能马上发送，只支持TCP/IP协议，不支持异步
	//MAKEWORD(2,2)可以同时接收和发送，支持多协议，支持异步
	WSADATA wsadata;
	if (0 != WSAStartup(wVersion, &wsadata))	//WSA:widows socket ansyc	windows异步套接字
	{
		return false;
	}
    return true;

}
bool close_socket()
{
    if (0 != WSACleanup())
	{
		return false;
	}
	return true;
}
bool create_tcp_socket(SOCKET* Socket )
{
    *Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*Socket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }
    return true;
}
bool create_udp_socket(SOCKET* Socket )
{
    *Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (*Socket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }
    return true;
}

bool bind_socket(SOCKET Socket, const char* ip, unsigned short port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (bind(Socket, (SOCKADDR*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}
bool listen_socket(SOCKET Socket, int backlog)
{
    if (listen(Socket, backlog) == SOCKET_ERROR) {
        closesocket(Socket);
        WSACleanup();
        return false;
    }
    return true;
}

bool accept_socket(SOCKET Socket, SOCKET* ConnectSocket, struct sockaddr_in* addr_in)
{
    int addr_len = sizeof(sockaddr_in);
    *ConnectSocket = accept(Socket, (SOCKADDR*)addr_in, &addr_len);
    // printf("%s",inet_ntoa(addr_in.sin_addr));
    if (*ConnectSocket == INVALID_SOCKET) {
        return false;
    }
    return true;
}

bool connect_socket(SOCKET Socket, struct sockaddr_in* addr_in)
{
    if (connect(Socket, (SOCKADDR*)addr_in, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        closesocket(Socket);
        WSACleanup();
        return false;
    }
    return true;
}

bool udp_send_socket(SOCKET Socket, const char* buffer, int buffer_size,sockaddr* address)
{
    int sent_bytes = sendto(Socket, buffer, buffer_size, 0, address, sizeof(sockaddr_in));
    if (sent_bytes == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool send_socket(SOCKET Socket, const char* buffer, int buffer_size)
{
    int sent_bytes = send(Socket, buffer, buffer_size, 0);
    if (sent_bytes == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool recv_socket(SOCKET Socket, char* buffer, int buffer_size)
{
    int recv_bytes = recv(Socket, buffer, buffer_size, 0);
    if (recv_bytes == SOCKET_ERROR) {
        return false;
    }
    return true;
}
#endif


#endif