#pragma once
#include "hub.h"
#include <iostream>
#include <string>
#include <vector>
#include <mutex>

#include "sqlite3.h"
#pragma comment(lib, "sqlite3.lib")		//���ݿ�
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")		//socket���
using namespace std;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define BUF_LENGTH 1024
#define REQ_QUEUE_LENGTH 4

class Hub;
class EndPoint {				//�˿ڷ�����
public:
	EndPoint(int uid, sqlite3*& db, Hub& hub);
	~EndPoint();

	int init();					//��ʼ�������ض˿�ֵ��0Ϊ����
	void process();				//����ɾ����endpoint

	bool isOnline() const { return online; }
	int getUserID() const { return userID; }
	string getUserName() const { return userName; }
	int getPort() const { return port; }

private:
	Hub &hub;					//�ܷ������ˣ����ã�
	sqlite3*& db;				//���ݿ⣨���ã�

	//���������Ϣ
	int port;					//�˿ں�
	string ip;					//ip��ַ
	SOCKET endpointSocket;
	SOCKET connectSocket;
	bool running;
	bool online;
	char buf[BUF_LENGTH];

	//�û������Ϣ
	int userID;					//�û�ID��ȫ��Ψһ��ϵͳ����
	string userName;			//�û�����ȫ��Ψһ
	mutex mtx;
	condition_variable cv;
	bool timing;				//��¼timerFunc()�����Ƿ�������

	void timerFunc();			//��ʱ
	void listenFunc();			//����

	void getUserList();						//��ȡ�����û���Ϣ(ʵ������hub�е���)
	void getPokemonList(int uid);			//�����û�id������ӵ�е�С����
	void name2Poke(string uname);			//�����û���������ӵ�е�С����
	void battleResult(bool battleGame, bool victory, int pid, int vspokekind, int vspokelv, int exp);//����ս�����

	void updateM1();						//���³����������
	void updateM2();						//���¸߼��������

	int exp2lv(int curexp);					//���ݾ���ֵȷ���ȼ�
	int lv2exp(int curlv);					//���ݵȼ�ȷ������ֵ
};