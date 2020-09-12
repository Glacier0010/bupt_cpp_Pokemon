#include <map>
#include <string>
#include <ctime>
#include "hub.h"
#include "port.h"
#include "Pokemon.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

/*����ʵ��Hub���Ա����*/
Hub::~Hub() {
	mtx.lock();				//ɾ������endpoints
	while (eps.size()) {
		EndPoint* p = eps.back();
		delete p;
		eps.pop_back();
	}
	mtx.unlock();

	closesocket(hubSocket);
	closesocket(connectSocket);

	while (WSACleanup() != -1);
}
int Hub::getM1(int uid) {		//�����������
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id FROM Pokemon WHERE userID = " + to_string(uid) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
	}
	else {
		if (nRow < 5)			//�޻���
			return 0;
		else if (nRow < 10)		//ͭ����
			return 1;
		else if (nRow < 15)		//������
			return 2;
		else return 3;			//�����
	}
	sqlite3_free_table(sqlResult);
	return 0;
}
int Hub::getM2(int uid) {		//�߼��������
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id FROM Pokemon WHERE userID = " + to_string(uid) + " AND level = 15;";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
	}
	else {
		if (nRow < 2)			//�޻���
			return 0;
		else if (nRow < 5)		//ͭ����
			return 1;
		else if (nRow < 7)		//������
			return 2;
		else return 3;			//�����
	}
	sqlite3_free_table(sqlResult);
	return 0;
}

void Hub::init() {
	cout << "Hub: Init database...";
	if (sqlite3_open("server.db", &db)) {
		cout << "\nHub: Can NOT open database: " << sqlite3_errmsg(db) << endl;
		return;
	}
	char* errMsg;
	//�����û��Ļ�����Ϣ��id/�˺�/����/����)
	string sql = "CREATE TABLE User(";
	sql += "id integer primary key autoincrement, ";		//�û�id��ϵͳ�ڲ�ʹ�ã����ڴ������ݣ�ȫ��Ψһ��
	sql += "name text unique not null, ";					//�û����ƣ�ȫ��Ψһ��
	sql += "password text not null, ";						//�û�����
	sql += "mailbox text not null, ";						//�û�����
	sql += "victoryGame integer, ";							//ʤ����ս����
	sql += "allGame integer, ";								//�ܶ�ս����
	sql += "PokeNumMedal integer, ";						//�����������
	sql += "superPokeMedal integer";						//�߼��������
	sql += ");";
	if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
	}
	//���������
	sql = "create table Pokemon(";
	sql += "id integer primary key autoincrement, ";				//ÿֻ�����һ�޶���id
	sql += "userID integer not null, ";								//����id
	sql += "type text, ";											//���ࣨ�ı���
	sql += "kind integer not null, ";								//�������ͣ�ȡֵ1~12����ϸ�μ�˵���ļ���
	sql += "name text, ";											//�Լ������֣���������͹ҹ���
	sql += "level integer not null, ";								//�ȼ�ֵ
	sql += "exp integer not null, ";								//����ֵ
	sql += "attack integer, ";										//������
	sql += "defence integer, ";										//������
	sql += "hp integer, ";											//����ֵ
	sql += "atk_interval integer";									//�������
	sql += ");";
	if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
	}
	cout << "Done.\n";

	//��ʼ��socket DLL
	cout << "Hub: Init socket DLL...";
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
		cout << "\nHub: Init network protocol failed.\n";
		return;
	}
	cout << "Done.\n";

	//��ʼ���������˵�socket
	cout << "Hub: Init hub socket...";
	hubSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hubSocket == INVALID_SOCKET) {
		cout << "\nHub: Init socket failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		return;
	}
	cout << "Done.\n";

	//������˽���һ��Address������Э�顢IP��ַ�Ͷ˿ں�
	sockaddr_in hubAddr;
	hubAddr.sin_family = AF_INET;
	hubAddr.sin_port = htons(PORT);
	hubAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//��socket��address
	cout << "Hub: Socket binding...";
	// namespace conflicts: thread::bind and global::bind
	if (::bind(hubSocket, (sockaddr*)&hubAddr, sizeof(hubAddr)) == SOCKET_ERROR) {
		cout << "\nHub: Socket bind failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		return;
	}
	cout << "Done.\n";

	//�����������������������ˣ��򱨴�
	cout << "Hub: Socket listen...";
	if (listen(hubSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR) {
		cout << WSAGetLastError();
		cout << "\nHub: Socket listen failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		return;
	}
	cout << "Done.\n";

	//�ɹ�����
	cout << "\nHub: Hub is running at " << PORT << endl;
	cout << "Press any key to stop hub.\n\n";

	//��ʼ���߳�
	running = true;
	thread listenThread(&Hub::listenFunc, this);
	thread terminateThread(&Hub::terminateFunc, this);
	listenThread.join();
	terminateThread.join();

	//�ݻ�����endpoints
	mtx.lock();
	while (eps.size()) {
		auto p = eps.back();
		delete p;
		eps.pop_back();
	}
	mtx.unlock();

	closesocket(hubSocket);
	WSACleanup();
	sqlite3_close(db);

	cout << "\nHub: Hub stopped.\n";					//��ɳ�ʼ��
}
Hub& Hub::getInstance() {
	static Hub result; 
	return result;
}
string Hub::getAllUser() {
	struct userInfo {
		string name;
		bool online;
		int vicG;
		int allG;
		int M1;
		int M2;
	};
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id, name, victoryGame, allGame FROM User;";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		strcpy_s(buf, "Reject: Hub database error.\n");
		sqlite3_free(errMsg);
	}

	map<int, userInfo> userMap;
	for (int i = 0; i < nRow; i++) {									//�������ݿ����������Ϣ����Ϊmap
		userInfo temp = { sqlResult[4 * i + 5], false, atoi(sqlResult[4 * i + 6]), atoi(sqlResult[4 * i + 7]) };
		temp.M1 = getM1(stoi(sqlResult[4 * i + 4]));
		temp.M2 = getM2(stoi(sqlResult[4 * i + 4]));
		userMap.insert(make_pair(stoi(sqlResult[4 * i + 4]), temp));	//id��temp����һ��
	}

	sqlite3_free_table(sqlResult);

	string result;
	mtx.lock();
	for (EndPoint* endpoint : eps) {
		userMap[endpoint->getUserID()].online = true;					//����û����ߣ���online����
	}
	mtx.unlock();

	for (auto& user : userMap) {
		if (user.second.online) 										//����û����ߣ������ǰ�棨y/n��ʾonline or not��
			result = to_string(user.first) + ' ' + user.second.name + " y " + to_string(user.second.vicG) + ' ' + to_string(user.second.allG) + ' ' + to_string(user.second.M1) + ' ' + to_string(user.second.M2) + '\n' + result;
		else 															//����û������ߣ���ź���
			result += to_string(user.first) + ' ' + user.second.name + " n " + to_string(user.second.vicG) + ' ' + to_string(user.second.allG) + ' ' + to_string(user.second.M1) + ' ' + to_string(user.second.M2) + '\n';
	}
	return result;
}

void Hub::registerNewUser(const string& username, const string& password, const string& mailbox) {
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	int population = 0;							//��ǰע���û���������д���û���ע�ᾫ���userID��
	string sql = "SELECT id FROM User;";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		strcpy_s(buf, "Reject: Hub database error.\n");
		sqlite3_free(errMsg);
	}
	else {
		population = nRow;
	}
	sql = "SELECT name FROM User WHERE name = '" + username + "';";	//�����ݿ�Ѱ�Ҹ��û���
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		strcpy_s(buf, "Reject: Hub database error.\n");
		sqlite3_free(errMsg);
	}
	else {
		if (nRow == 0) {													//�Ҳ�����˵�����û���δ��ע���
			population++;
			sql = "INSERT INTO User VALUES(NULL, '"							//����id����ΪNULL�ͻ��Զ�����
				+ username + "', '" 
				+ password + "', '" 
				+ mailbox + "', 0, 0, 0, 0);";
				
			char* errMsg;
			if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
				cout << "Hub: Sqlite3 error: " << errMsg << endl;
				sqlite3_free(errMsg);
				strcpy_s(buf, "Reject: Hub database error.\n");
			}
			else {
				cout << "Hub: Add user: " << username << " / password: " << password << " / mailbox: " << mailbox << endl;
				sql = "";
				srand(time(NULL));
				for (int i = 0; i < 3; i++) {				//�ɹ�ע����������3������						
					int kindIndex = rand() % 12;
					Pokemon temp(kindIndex);
					sql += "INSERT INTO Pokemon VALUES(NULL, "
						+ to_string(population) + ", '"
						+ to_string(temp.getType()) + "', "
						+ to_string(temp.getKind()) + ", '"
						+ nameDict[temp.getKind()] + "', "
						+ to_string(temp.getLv()) + ", "
						+ to_string(temp.getExp()) + ", "
						+ to_string(temp.getAtk()) + ", "
						+ to_string(temp.getDfc()) + ", "
						+ to_string(temp.getHp()) + ", "
						+ to_string(temp.getATKINT()) + "); ";
				}
				if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
					cout << "Hub: Sqlite3 error: " << errMsg << endl;
					sqlite3_free(errMsg);
					strcpy_s(buf, "Reject: Hub database error.\n");
				}
				else {
					cout << "Hub: Add 3 primary pokemons successfully!\n";
					strcpy_s(buf, "Accepted.\n");
				}
			}
		}
		else {									//�ҵ������ģ����û����Ѿ���ע��
			cout << "Hub: Register: username '" << username << "' has already been taken.\n";
			strcpy_s(buf, "Reject: Username has been taken!\n");
		}
		sqlite3_free_table(sqlResult);
	}	
	send(connectSocket, buf, BUF_LENGTH, 0);
}
void Hub::login(const string& username, const string& password) {	//�ж��˺������Ƿ�ƥ��	
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id FROM User WHERE name = '" + username + "' AND password = '" + password + "';";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		strcpy_s(buf, "Reject: Hub database error.\n");
		sqlite3_free(errMsg);
	}
	else {
		if (nRow == 0) {							//�˺����벻ƥ��
			cout << "Hub: Login: username '" << username << "' and password '" << password << "' mismatch.\n";
			strcpy_s(buf, "Reject: Username and password dismatch.\n");
		}
		else {										//������ȷ��Ȼ�����û�������񡢶�Ӧendpoint���������Ϊע��ʱ������endpoint��
			bool userOnline = false;
			mtx.lock();
			int id = atoi(sqlResult[1]);			//sqlResult[0] == "id"�ֶα�ʶ, sqlResult[1] == �ҵ����û�id
			for (EndPoint* endpoint : eps) {
				if (endpoint->getUserID() == id) {	//����Ѿ�������endpoint				
					if (endpoint->isOnline()) {		//����Ѿ����ߣ��򲵻�
						strcpy_s(buf, "Reject: This account has already been online.\n");
						cout << "Login Reject: This account has already been online.\n";
					}
					else {							//��������ߣ�����״̬�����򷵻ض˿ں�
						strcpy_s(buf, to_string(endpoint->getPort()).c_str());
						cout << "Login successfully.\n";
					}
					break;
					userOnline = true;
				}
			}
			mtx.unlock();
			if (!userOnline) {						//���˺���ע�ᵫû�н���endpoint��������һ��endpoint			
				auto p = new EndPoint(id, db, *this);
				int endpointPort = p->init();
				if (endpointPort == 0) {			//������ִ�����ɾ�����endpoint				
					delete p;
					strcpy_s(buf, "Reject: Hub endpoint error.\n");
				}
				else {								//û�����⣬����뵽endpoints���ͥ��
					lock_guard<mutex> lock(mtx);
					eps.push_back(p);
					strcpy_s(buf, to_string(endpointPort).c_str());
					thread th(&Hub::monitorFunc, this, p);
					th.detach();
				}
			}
		}
		sqlite3_free_table(sqlResult);
	}
	send(connectSocket, buf, BUF_LENGTH, 0);
}

void Hub::listenFunc() {
	while (running) {
		sockaddr_in clientAddr;
		int clientAddrLength = sizeof(clientAddr);
		connectSocket = accept(hubSocket, (sockaddr*)&clientAddr, &clientAddrLength);
		if (connectSocket == INVALID_SOCKET) {
			if (running) {
				cout << "Hub: Link to client failed.\n";
			}
			closesocket(connectSocket);
			break;
		}
		cout << "Hub: " << inet_ntoa(clientAddr.sin_addr) << " connected.\n";	//��ʾ�ͻ��˵���ַ

		recv(connectSocket, buf, BUF_LENGTH, 0);		//��ʼ����ͻ��˷�����request
		vector<string> strs;
		string t="";
		for (auto c : buf) {							//��buf�����Կո�Ϊ�ָ�����Ϊ��ͬ��ָ��						
			if ((c != ' ') && (c!='\n'))
				t += c;
			else {
				strs.push_back(t);
				if (c == ' ')
					t = "";
				else break;								//'\n'��Ϊ������־
			}
		}
		if (strs.size() == 1 && strs[0].length() == 0) {//û��request����������Ҳ��ͻ����ѹرգ�
		}
		else if (strs.size() <= 2) {					//����̫�٣�����3�����Ĵ���			
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "register") {				//����ע��
			registerNewUser(strs[1], strs[2], strs[3]);
		}
		else if (strs[0] == "login")					//�����¼
			login(strs[1], strs[2]);
		else {											//���������request
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
		}
		closesocket(connectSocket);
	}
}
void Hub::terminateFunc() {
	getchar();											//�����ֹͣ������������
	running = false;
	closesocket(hubSocket);								//�ر�Hub socket
}
void Hub::monitorFunc(EndPoint* endpoint) {
	endpoint->process();
	mtx.lock();
	for (int i = 0; i < eps.size(); i++) {
		if (eps[i] == endpoint) {
			eps.erase(eps.begin() + i);
			delete endpoint;
			break;
		}
	}
	mtx.unlock();
}
