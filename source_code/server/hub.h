#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include "port.h"

#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")		//���ݿ�
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")		//socket���

#pragma warning(disable:4996)			//����һ��(��ʾ�Ľ������)ʵ���޷����error 4996
//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define BUF_LENGTH 1024
#define REQ_QUEUE_LENGTH 4
class EndPoint;
class Hub {					//�ܷ�������
public:
	void init();			//��ʼ�����ݿ��socket
	static Hub& getInstance();
	string getAllUser();	//��ѯ�����û���Ϣ�������������

private:
	const int PORT = 7500;	//���������Ϣ
	SOCKET hubSocket;
	SOCKET connectSocket;
	bool running;
	char buf[BUF_LENGTH];	//������

	sqlite3* db;			//��Ϸ���ݿ�
	vector<EndPoint*> eps;	//���еĶ˿ڷ�������
	mutex mtx;				//���߳����

	Hub() {};
	~Hub();
	Hub(Hub const&) = delete;
	Hub(Hub&&) = delete;
	Hub& operator=(Hub const&) = delete;

	void registerNewUser(const string& username, const string& password, const string& mailbox);//���û�ע��
	void login(const string& username, const string& password);									//��¼
	int getM1(int uid);		//��������������
	int getM2(int uid);		//����߼��������

	void listenFunc();
	void terminateFunc();
	void monitorFunc(EndPoint* endpoint);
};

