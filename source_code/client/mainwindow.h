#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTime>

#include "userlist.h"
#include "pokemonlist.h"
#include "cfdialog.h"
#include "smpdialog.h"
#include "fffdialog.h"
#include "dialog.h"
#include "rdialog.h"
#include "parameter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    RDialog *rdialog;
    UserList *ul;
    QTcpSocket *client;

    QTime syst0 = QTime::currentTime();

    int state = LOGIN;
    int PORT = 7500;
    int curuid = 1;                     //尝试查询的ID
    QString curname;                    //尝试登录的用户名
    QString thisname = "";              //当前用户名字

    QString enemyPoke;                  //敌方精灵信息 格式：y(/n)_kind_level\n
    QString allMyPoke;                  //我方所有精灵信息
    QString myPoke;                     //我方出战精灵信息 格式：id_kind_level\n
    QString retnow;                     //当前战斗结果 格式：1(/0)_1(/0)_myPokemonID_vsPokeKind_vsPokeLv_exp\n

    bool isValidStr(QString str);       //判断用户名/密码/邮箱格式是否正确
    void fightui();                     //对战界面
    void loginui();                     //登录界面

private slots:
    void getServerMsg();                //获取服务器端信息
    void on_pushButton_clicked();       //登录按钮
    void on_pushButton_2_clicked();     //注册按钮
    void logout();                      //登出
    void getUserInfo();                 //查看在线用户
    void choosefightPoke1();            //升级赛，选择对战精灵
    void choosefightPoke2();            //决斗赛，选择对战精灵

    void getPokeStr(QString);           //接受用户选择的对战精灵信息
    void getStr(QString);               //接受用户选择出战的我方精灵信息
    void getLoseStr(QString);           //接受用户选择送出的精灵信息
    void getRet(QString);               //接受战斗结果
};

#endif // MAINWINDOW_H
