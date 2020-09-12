#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include "port.h"

#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")		//数据库
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")		//socket编程

#pragma warning(disable:4996)			//用下一句(提示的解决方法)实在无法解决error 4996
//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define BUF_LENGTH 1024
#define REQ_QUEUE_LENGTH 4
class EndPoint;
class Hub {					//总服务器端
public:
	void init();			//初始化数据库和socket
	static Hub& getInstance();
	string getAllUser();	//查询所有用户信息，包括在线与否

private:
	const int PORT = 7500;	//网络相关信息
	SOCKET hubSocket;
	SOCKET connectSocket;
	bool running;
	char buf[BUF_LENGTH];	//缓冲区

	sqlite3* db;			//游戏数据库
	vector<EndPoint*> eps;	//所有的端口服务器端
	mutex mtx;				//多线程相关

	Hub() {};
	~Hub();
	Hub(Hub const&) = delete;
	Hub(Hub&&) = delete;
	Hub& operator=(Hub const&) = delete;

	void registerNewUser(const string& username, const string& password, const string& mailbox);//新用户注册
	void login(const string& username, const string& password);									//登录
	int getM1(int uid);		//计算宠物个数徽章
	int getM2(int uid);		//计算高级宠物徽章

	void listenFunc();
	void terminateFunc();
	void monitorFunc(EndPoint* endpoint);
};

