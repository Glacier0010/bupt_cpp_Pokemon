#pragma once
#include "hub.h"
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")		//数据库
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")		//socket编程
using namespace std;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define BUF_LENGTH 1024
#define REQ_QUEUE_LENGTH 4

class Hub;
class EndPoint {				//端口服务器
public:
	EndPoint(int uid, sqlite3*& db, Hub& hub);
	~EndPoint();

	int init();					//初始化，返回端口值，0为出错
	void process();				//返回删除该endpoint

	bool isOnline() const { return online; }
	int getUserID() const { return userID; }
	string getUserName() const { return userName; }
	int getPort() const { return port; }

private:
	Hub &hub;					//总服务器端（引用）
	sqlite3*& db;				//数据库（引用）

	//网络相关信息
	int port;					//端口号
	string ip;					//ip地址
	SOCKET endpointSocket;
	SOCKET connectSocket;
	bool running;
	bool online;
	char buf[BUF_LENGTH];

	//用户相关信息
	int userID;					//用户ID，全局唯一，系统分配
	string userName;			//用户名，全局唯一
	mutex mtx;
	condition_variable cv;
	bool timing;				//记录timerFunc()函数是否运行中

	void timerFunc();			//计时
	void listenFunc();			//监听

	void getUserList();						//获取所有用户信息(实际上在hub中调用)
	void getPokemonList(int uid);			//根据用户id找其所拥有的小精灵
	void name2Poke(string uname);			//根据用户名找其所拥有的小精灵
	void battleResult(bool battleGame, bool victory, int pid, int vspokekind, int vspokelv, int exp);//处理战斗结果

	void updateM1();						//更新宠物个数徽章
	void updateM2();						//更新高级宠物徽章

	int exp2lv(int curexp);					//根据经验值确定等级
	int lv2exp(int curlv);					//根据等级确定经验值
};