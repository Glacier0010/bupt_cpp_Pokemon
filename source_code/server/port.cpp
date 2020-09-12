#include <conio.h>
#include <map>
#include "hub.h"
#include "port.h"

using namespace std;

/*以下实现EndPoint类成员函数*/
void EndPoint::timerFunc() {
	using namespace std::chrono_literals;
	unique_lock<mutex> lock(mtx);
	if (!cv.wait_for(lock, 10min, [this] { return online; })) {	//用户长时间（10min）离线就直接下线	
		running = false;
		timing = false;
		closesocket(endpointSocket);
		cout << "Endpoint[" << userID << "]: Time up.\n";
	}
	else
		timing = false;	
}
void EndPoint::listenFunc() {									//监听函数
	sockaddr_in clientAddr; 
	int clientAddrLength = sizeof(clientAddr);
	connectSocket = accept(endpointSocket, (sockaddr*)&clientAddr, &clientAddrLength);
	while (timing) {
		unique_lock<mutex> lock(mtx);
		online = true;
		lock.unlock();
		cv.notify_one();
		lock.lock();
	}
	if (connectSocket == INVALID_SOCKET) {
		return;
	}
	int ret = recv(connectSocket, buf, BUF_LENGTH, 0);
	
	while (ret != 0 && ret != SOCKET_ERROR && running) {
		vector<string> strs;
		string t;
		for (auto c : buf) {							//将buf内容以空格为分隔，分为不同的指令						
			if ((c != ' ') && (c != '\n'))
				t += c;
			else {
				strs.push_back(t);
				if (c == ' ')
					t = "";
				else break;								//'\n'作为结束标志
			}
		}
														//以下对不同的指令进行操作
		if (strs[0] == "logout") {						//退出登录
			running = false;
			cout << "Endpoint[" << userID << "]: Logging out.\n";
		}
		else if (strs[0] == "getUserList")	{			//获得所有用户名单
			getUserList();
			cout << "Endpoint[" << userID << "]: Getting User List.\n";
		}
		else if (strs[0] == "getPokemonList" && strs.size() == 2) {	//按照用户id查看其所拥有的精灵
			getPokemonList(stoi(strs[1]));	
			cout << "Endpoint[" << userID << "]: Getting Pokemon List of userid=" << strs[1] << endl;
		}
		else if (strs[0] == "nameToPoke" && strs.size() == 2) {		//按照用户name查看其精灵信息（准备对战时）
			name2Poke(strs[1]);
			cout << "Endpoint[" << userID << "]: Getting user's Pokemon list of username=" << strs[1] << endl;
		}
		else if (strs[0] == "battleResult" && strs.size() > 2) {	//按照战斗结果修改信息
			bool t0 = (strs[1] == "1") ? true : false;				//是否为决斗赛
			bool t1 = (strs[2] == "1") ? true : false;				//是否胜利
			cout << "Endpoint[" << userID << "]: Getting battle result." << endl;
			battleResult(t0, t1, stoi(strs[3]), stoi(strs[4]), stoi(strs[5]), stoi(strs[6]));
		}
		else {
			cout << "Endpoint[" << userID << "]: Invalid request.\n";
			strcpy_s(buf, "Reject: Invalid request.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
		}
		if (running)
			ret = recv(connectSocket, buf, BUF_LENGTH, 0);
	}
	if (!running) ;
	else if (ret == SOCKET_ERROR || ret == 0)
		cout << "Endpoint[" << userID << "]: Client unexpected offline, start timing.\n";
	else
		cout << "Endpoint[" << userID << "]: User log out.\n";
	closesocket(connectSocket);
}
void EndPoint::battleResult(bool battleGame, bool victory, int pid, int vsPokeKind, int vsPokeLv, int exp) {
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	int vg, ag;
	sql = "SELECT victoryGame, allGame FROM User where id=" + to_string(userID) + ';';		//用户对战次数+1，胜利次数更新
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
		cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
		strcpy_s(buf, "Reject: Server error.\n");
		send(connectSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else {
		vg = stoi(sqlResult[2]);
		if (victory) vg++;
		ag = stoi(sqlResult[3]) + 1;
		sql = "UPDATE User SET victoryGame=" + to_string(vg) + ", allGame=" + to_string(ag) + " WHERE id=" + to_string(userID) + ";";
		if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
			strcpy_s(buf, "Reject: Server error.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
			return;
		}
		else
			cout << "Endpoint[" << userID << "]: Update user victory record successfully.\n";
	}
	if (victory) {												//如果胜利，则更新小精灵经验，判断是否升级
		sql = "SELECT exp FROM Pokemon where id=" + to_string(pid) + ';';
		if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
			strcpy_s(buf, "Reject: Server error.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
			return;
		}
		else {
			int curexp = stoi(sqlResult[1]) + exp;
			int curlv = exp2lv(curexp);
			sql = "UPDATE Pokemon SET exp=" + to_string(curexp) + ", level=" + to_string(curlv) + " WHERE id=" + to_string(pid) + ";";
			if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
				sqlite3_free(errMsg);
				cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
				strcpy_s(buf, "Reject: Server error.\n");
				send(connectSocket, buf, BUF_LENGTH, 0);
				return;
			}
			else
				cout << "Endpoint[" << userID << "]: Update Pokemon exp and level successfully.\n";
		}
	}

	if (battleGame) {											//如果是决斗赛，则还要增减精灵
		if (victory) {											//增加一只小精灵
			sql = "INSERT INTO Pokemon (id, userID, kind, level, exp) VALUES(NULL, "
				+ to_string(userID) + ", "
				+ to_string(vsPokeKind) + ", "
				+ to_string(vsPokeLv) + ", "
				+ to_string(lv2exp(vsPokeLv)) + "); ";		
			if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
				cout << "Endpoint[" << userID << "]: Sqlite3 error: " << errMsg << endl;
				sqlite3_free(errMsg);
				strcpy_s(buf, "Reject: Hub database error.\n");
				send(connectSocket, buf, BUF_LENGTH, 0);
				return;
			}
			else {
				cout << "Endpoint[" << userID << "]: Add 1 pokemon successfully!\n";
			}			
		}
		else {													//删除一只小精灵，如果没有小精灵了，就随机分配一只
			sql = "DELETE FROM Pokemon WHERE id=" + to_string(pid) + ";";
			if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
				cout << "Endpoint[" << userID << "]: Sqlite3 error: " << errMsg << endl;
				sqlite3_free(errMsg);
				strcpy_s(buf, "Reject: Hub database error.\n");
				send(connectSocket, buf, BUF_LENGTH, 0);
				return;
			}
			else {
				cout << "Endpoint[" << userID << "]: Delete 1 pokemon successfully!\n";
			}
			sql = "SELECT id FROM Pokemon WHERE userID=" + to_string(userID) + ';';
			if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
				sqlite3_free(errMsg);
				cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
				strcpy_s(buf, "Reject: Server error.\n");
				send(connectSocket, buf, BUF_LENGTH, 0);
				return;
			}
			else if (nRow == 0) {								//如果该用户没有精灵了
					srand(time(NULL));							//就随机送一只1级精灵
					sql = "INSERT INTO Pokemon (id, userID, kind, level, exp) VALUES(NULL, "
						+ to_string(userID) + ", "
						+ to_string(rand() % 12) + ", "
						+ to_string(1) + ", "
						+ to_string(0) + "); ";
					if (sqlite3_exec(db, sql.c_str(), 0, NULL, &errMsg) != SQLITE_OK) {
						cout << "Endpoint[" << userID << "]: Sqlite3 error: " << errMsg << endl;
						sqlite3_free(errMsg);
						strcpy_s(buf, "Reject: Hub database error.\n");
						send(connectSocket, buf, BUF_LENGTH, 0);
						return;
					}
			}
		}
	}
	updateM1();
	updateM2();
	sqlite3_free_table(sqlResult);	
	cout << "Endpoint[" << userID << "]: Battle result Accepted.\n";
}
void EndPoint::updateM1()	{					//更新徽章1
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id FROM Pokemon WHERE userID=" + to_string(userID) + ';';
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
		cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
		strcpy_s(buf, "Reject: Server error.\n");
		send(connectSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else {
		int m1 = 0;
		if (nRow >= 5 && nRow < 10)  m1 = 1;
		else if (nRow >= 10 && nRow < 15)	 m1 = 2;
		else if (nRow >= 15)	 m1 = 3;
		sql = "UPDATE User SET PokeNumMedal=" + to_string(m1) + " WHERE id=" + to_string(userID) + ";";
		if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
			strcpy_s(buf, "Reject: Server error.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
			return;
		}
	}
	sqlite3_free_table(sqlResult);
}
void EndPoint::updateM2() {						//更新徽章2
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql = "SELECT id FROM Pokemon WHERE level=15 AND userID=" + to_string(userID) + ';';
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
		cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
		strcpy_s(buf, "Reject: Server error.\n");
		send(connectSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else {
		int m2 = 0;
		if (nRow >= 2 && nRow < 5)  m2 = 1;
		else if (nRow >= 5 && nRow < 7)	 m2 = 2;
		else if (nRow >= 7)	 m2 = 3;
		sql = "UPDATE User SET superPokeMedal=" + to_string(m2) + " WHERE id=" + to_string(userID) + ";";
		if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
			sqlite3_free(errMsg);
			cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
			strcpy_s(buf, "Reject: Server error.\n");
			send(connectSocket, buf, BUF_LENGTH, 0);
			return;
		}
	}
	sqlite3_free_table(sqlResult);
}
int EndPoint::exp2lv(int exp) {							//经验值转换为等级
	if (exp < 108) return 1;
	int r = 2;
	while (2 * r * r + 200 * r - 300 <= exp)
		r++;
	if (r > 15) return 15;
	return r - 1;
}
int EndPoint::lv2exp(int lv) {							//等级转经验值
	if (lv == 1) return 0;
	return 2 * lv * lv + 200 * lv - 300;
}
void EndPoint::name2Poke(string uname) {				//根据用户名找小精灵信息
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT id FROM User where name='" + uname + "';";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
		cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
		strcpy_s(buf, "Reject: Server error.\n");
		send(connectSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else {
		getPokemonList(stoi(sqlResult[1]));		
	}
	sqlite3_free_table(sqlResult);
}
void EndPoint::getUserList(){
	strcpy_s(buf, hub.getAllUser().c_str());
	send(connectSocket, buf, BUF_LENGTH, 0);
}
void EndPoint::getPokemonList(int uid) {
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;
	string sql;
	sql = "SELECT id, kind, level FROM Pokemon where userID=" + to_string(uid) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		sqlite3_free(errMsg);
		cout << "Endpoint[" << userID << "]: Reject: Server error.\n";
		strcpy_s(buf, "Reject: Server error.\n");
		send(connectSocket, buf, BUF_LENGTH, 0);
		return;
	}
	else {
		string result = "";
		for (int i = 0; i < nRow; i++) {
			result += sqlResult[3 * (i + 1)];		// id
			result += ' ';
			result += sqlResult[3 * (i + 1) + 1];	// kind
			result += ' ';
			result += sqlResult[3 * (i + 1) + 2];	// level
			result += '\n';
		}
		strcpy_s(buf, result.c_str());
		send(connectSocket, buf, BUF_LENGTH, 0);
	}	
	sqlite3_free_table(sqlResult);
}

EndPoint::EndPoint(int uid, sqlite3*& db, Hub& hub):userID(uid),db(db),hub(hub),port(0),running(false) {
}
EndPoint::~EndPoint() {
	running = false;
	while (timing)	{
		unique_lock<mutex> lock(mtx);
		online = true;
		lock.unlock();
		cv.notify_one();
		lock.lock();
	}
	closesocket(endpointSocket);
	if (port)
		cout << "Endpoint[" << userID << "]: Endpoint stoped at " << port << endl;
}

int EndPoint::init() {
	char** sqlResult;
	int nRow;
	int nColumn;
	char* errMsg;					//根据id找到该用户的用户名
	string sql = "SELECT name FROM User where id=" + to_string(userID) + ";";
	if (sqlite3_get_table(db, sql.c_str(), &sqlResult, &nRow, &nColumn, &errMsg) != SQLITE_OK) {
		cout << "Endpoint[" << userID << "]: Sqlite3 error: " << errMsg << endl;
		sqlite3_free(errMsg);
		return 0;
	}
	else if (nRow == 0)	{			//如果找不到，说明数据库有问题
		cout << "Endpoint[" << userID << "]: Database content error.\n";
		sqlite3_free_table(sqlResult);
		return 0;
	}
	else {
		userName = sqlResult[1];	//sqlResult[0]为name字段标识，sqlResult[1]为找到的user name
		sqlite3_free_table(sqlResult);
	}

	endpointSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (endpointSocket == INVALID_SOCKET) {
		cout << "Endpoint[" << userID << "]: Init socket failed.\n";
		closesocket(endpointSocket);
		return 0;
	}

	sockaddr_in endpointAddr;
	endpointAddr.sin_family = AF_INET;
	endpointAddr.sin_port = htons(0);	//windows自动帮助分配一个端口
	endpointAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	//绑定socket和地址
	cout << "Endpoint[" << userID << "]: Socket binding...";
	if (::bind(endpointSocket, (sockaddr*)&endpointAddr, sizeof(endpointAddr)) == SOCKET_ERROR)	{
		cout << "Endpoint[" << userID << "]: Socket bind failed.\n";
		closesocket(endpointSocket);
		return 0;
	}

	int endpointAddrLength = sizeof(endpointAddr);
	getsockname(endpointSocket, (sockaddr*)&endpointAddr, &endpointAddrLength);
	port = ntohs(endpointAddr.sin_port);

	//如果请求队列满了，则客户端得到错误信息
	cout << "\nEndpoint[" << userID << "]: Socket listen...";
	if (listen(endpointSocket, REQ_QUEUE_LENGTH) == SOCKET_ERROR) {
		cout << WSAGetLastError();
		cout << "Endpoint[" << userID << "]: Socket listen failed.\n";
		closesocket(endpointSocket);
		return 0;
	}

	//成功listen
	cout << "\nEndpoint[" << userID << "]: Endpoint is running at " << port << endl;
	running = true; 
	return port;				//返回端口号，0表示未分配有错误
}
void EndPoint::process() {
	while (running)	{
		online = false;
		timing = true;

		thread timerThread(&EndPoint::timerFunc, this);
		thread listenThread(&EndPoint::listenFunc, this);
		timerThread.join();
		listenThread.join();
	}
}

