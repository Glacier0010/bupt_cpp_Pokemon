#include <map>
#include <string>
#include <ctime>
#include "hub.h"
#include "port.h"
#include "Pokemon.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

/*以下实现Hub类成员函数*/
Hub::~Hub() {
	mtx.lock();				//删除所有endpoints
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
int Hub::getM1(int uid) {		//宠物个数徽章
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
		if (nRow < 5)			//无徽章
			return 0;
		else if (nRow < 10)		//铜徽章
			return 1;
		else if (nRow < 15)		//银徽章
			return 2;
		else return 3;			//金徽章
	}
	sqlite3_free_table(sqlResult);
	return 0;
}
int Hub::getM2(int uid) {		//高级宠物徽章
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
		if (nRow < 2)			//无徽章
			return 0;
		else if (nRow < 5)		//铜徽章
			return 1;
		else if (nRow < 7)		//银徽章
			return 2;
		else return 3;			//金徽章
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
	//创建用户的基础信息表（id/账号/密码/邮箱)
	string sql = "CREATE TABLE User(";
	sql += "id integer primary key autoincrement, ";		//用户id（系统内部使用，便于处理数据；全局唯一）
	sql += "name text unique not null, ";					//用户名称（全局唯一）
	sql += "password text not null, ";						//用户密码
	sql += "mailbox text not null, ";						//用户邮箱
	sql += "victoryGame integer, ";							//胜利对战局数
	sql += "allGame integer, ";								//总对战局数
	sql += "PokeNumMedal integer, ";						//宠物个数徽章
	sql += "superPokeMedal integer";						//高级宠物徽章
	sql += ");";
	if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
	}
	//创建精灵表
	sql = "create table Pokemon(";
	sql += "id integer primary key autoincrement, ";				//每只精灵独一无二的id
	sql += "userID integer not null, ";								//主人id
	sql += "type text, ";											//种类（文本）
	sql += "kind integer not null, ";								//具体类型（取值1~12，详细参见说明文件）
	sql += "name text, ";											//自己的名字（与具体类型挂钩）
	sql += "level integer not null, ";								//等级值
	sql += "exp integer not null, ";								//经验值
	sql += "attack integer, ";										//攻击力
	sql += "defence integer, ";										//防御力
	sql += "hp integer, ";											//生命值
	sql += "atk_interval integer";									//攻击间隔
	sql += ");";
	if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
	}
	cout << "Done.\n";

	//初始化socket DLL
	cout << "Hub: Init socket DLL...";
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
		cout << "\nHub: Init network protocol failed.\n";
		return;
	}
	cout << "Done.\n";

	//初始化服务器端的socket
	cout << "Hub: Init hub socket...";
	hubSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (hubSocket == INVALID_SOCKET) {
		cout << "\nHub: Init socket failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		return;
	}
	cout << "Done.\n";

	//给服务端建立一个Address，包括协议、IP地址和端口号
	sockaddr_in hubAddr;
	hubAddr.sin_family = AF_INET;
	hubAddr.sin_port = htons(PORT);
	hubAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//绑定socket和address
	cout << "Hub: Socket binding...";
	// namespace conflicts: thread::bind and global::bind
	if (::bind(hubSocket, (sockaddr*)&hubAddr, sizeof(hubAddr)) == SOCKET_ERROR) {
		cout << "\nHub: Socket bind failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		return;
	}
	cout << "Done.\n";

	//持续监听，如果请求队列满了，则报错
	cout << "Hub: Socket listen...";
	if (listen(hubSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR) {
		cout << WSAGetLastError();
		cout << "\nHub: Socket listen failed.\n";
		closesocket(hubSocket);
		WSACleanup();
		return;
	}
	cout << "Done.\n";

	//成功监听
	cout << "\nHub: Hub is running at " << PORT << endl;
	cout << "Press any key to stop hub.\n\n";

	//初始化线程
	running = true;
	thread listenThread(&Hub::listenFunc, this);
	thread terminateThread(&Hub::terminateFunc, this);
	listenThread.join();
	terminateThread.join();

	//摧毁所有endpoints
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

	cout << "\nHub: Hub stopped.\n";					//完成初始化
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
	for (int i = 0; i < nRow; i++) {									//将在数据库检索到的信息制作为map
		userInfo temp = { sqlResult[4 * i + 5], false, atoi(sqlResult[4 * i + 6]), atoi(sqlResult[4 * i + 7]) };
		temp.M1 = getM1(stoi(sqlResult[4 * i + 4]));
		temp.M2 = getM2(stoi(sqlResult[4 * i + 4]));
		userMap.insert(make_pair(stoi(sqlResult[4 * i + 4]), temp));	//id和temp绑定在一起
	}

	sqlite3_free_table(sqlResult);

	string result;
	mtx.lock();
	for (EndPoint* endpoint : eps) {
		userMap[endpoint->getUserID()].online = true;					//如果用户在线，则将online更新
	}
	mtx.unlock();

	for (auto& user : userMap) {
		if (user.second.online) 										//如果用户在线，则放在前面（y/n表示online or not）
			result = to_string(user.first) + ' ' + user.second.name + " y " + to_string(user.second.vicG) + ' ' + to_string(user.second.allG) + ' ' + to_string(user.second.M1) + ' ' + to_string(user.second.M2) + '\n' + result;
		else 															//如果用户不在线，则放后面
			result += to_string(user.first) + ' ' + user.second.name + " n " + to_string(user.second.vicG) + ' ' + to_string(user.second.allG) + ' ' + to_string(user.second.M1) + ' ' + to_string(user.second.M2) + '\n';
	}
	return result;
}

void Hub::registerNewUser(const string& username, const string& password, const string& mailbox) {
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	int population = 0;							//当前注册用户数（便于写新用户的注册精灵的userID）
	string sql = "SELECT id FROM User;";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		strcpy_s(buf, "Reject: Hub database error.\n");
		sqlite3_free(errMsg);
	}
	else {
		population = nRow;
	}
	sql = "SELECT name FROM User WHERE name = '" + username + "';";	//在数据库寻找该用户名
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Hub: Sqlite3 error: " << errMsg << endl;
		strcpy_s(buf, "Reject: Hub database error.\n");
		sqlite3_free(errMsg);
	}
	else {
		if (nRow == 0) {													//找不到，说明该用户名未被注册过
			population++;
			sql = "INSERT INTO User VALUES(NULL, '"							//主键id设置为NULL就会自动分配
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
				for (int i = 0; i < 3; i++) {				//成功注册后随机分配3个精灵						
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
		else {									//找到重名的，该用户名已经被注册
			cout << "Hub: Register: username '" << username << "' has already been taken.\n";
			strcpy_s(buf, "Reject: Username has been taken!\n");
		}
		sqlite3_free_table(sqlResult);
	}	
	send(connectSocket, buf, BUF_LENGTH, 0);
}
void Hub::login(const string& username, const string& password) {	//判断账号密码是否匹配	
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
		if (nRow == 0) {							//账号密码不匹配
			cout << "Hub: Login: username '" << username << "' and password '" << password << "' mismatch.\n";
			strcpy_s(buf, "Reject: Username and password dismatch.\n");
		}
		else {										//密码正确，然后检测用户在线与否、对应endpoint建立与否（因为注册时不建立endpoint）
			bool userOnline = false;
			mtx.lock();
			int id = atoi(sqlResult[1]);			//sqlResult[0] == "id"字段标识, sqlResult[1] == 找到的用户id
			for (EndPoint* endpoint : eps) {
				if (endpoint->getUserID() == id) {	//如果已经建立了endpoint				
					if (endpoint->isOnline()) {		//如果已经在线，则驳回
						strcpy_s(buf, "Reject: This account has already been online.\n");
						cout << "Login Reject: This account has already been online.\n";
					}
					else {							//如果不在线（正常状态），则返回端口号
						strcpy_s(buf, to_string(endpoint->getPort()).c_str());
						cout << "Login successfully.\n";
					}
					break;
					userOnline = true;
				}
			}
			mtx.unlock();
			if (!userOnline) {						//该账号已注册但没有建立endpoint，则增加一个endpoint			
				auto p = new EndPoint(id, db, *this);
				int endpointPort = p->init();
				if (endpointPort == 0) {			//如果出现错误，则删除这个endpoint				
					delete p;
					strcpy_s(buf, "Reject: Hub endpoint error.\n");
				}
				else {								//没有问题，则加入到endpoints大家庭里
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
		cout << "Hub: " << inet_ntoa(clientAddr.sin_addr) << " connected.\n";	//显示客户端的网址

		recv(connectSocket, buf, BUF_LENGTH, 0);		//开始处理客户端发来的request
		vector<string> strs;
		string t="";
		for (auto c : buf) {							//将buf内容以空格为分隔，分为不同的指令						
			if ((c != ' ') && (c!='\n'))
				t += c;
			else {
				strs.push_back(t);
				if (c == ' ')
					t = "";
				else break;								//'\n'作为结束标志
			}
		}
		if (strs.size() == 1 && strs[0].length() == 0) {//没有request，不作处理（也许客户端已关闭）
		}
		else if (strs.size() <= 2) {					//参数太少（少于3个）的错误			
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
		}
		else if (strs[0] == "register") {				//请求注册
			registerNewUser(strs[1], strs[2], strs[3]);
		}
		else if (strs[0] == "login")					//请求登录
			login(strs[1], strs[2]);
		else {											//其他错误的request
			cout << "Hub: Invalid request: " << buf << endl;
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
		}
		closesocket(connectSocket);
	}
}
void Hub::terminateFunc() {
	getchar();											//任意键停止服务器端运行
	running = false;
	closesocket(hubSocket);								//关闭Hub socket
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
