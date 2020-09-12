#include <QPalette>
#include <QMessageBox>
#include <QDialog>
#include <QString>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDebug>

#include <QAbstractSocket>      //Socket相关
#include <QHostAddress>

#include "mainwindow.h"

#include "ui_userlist.h"
#include "ui_mainwindow.h"
#include "ui_rdialog.h"
#include "ui_cfdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("My Pokemon!");         //设置窗口标题
    resize(1024,768);                      //重置窗口大小
    setAutoFillBackground(true);           //设置背景图片
    QPalette pal;
    QPixmap pixmap("./photo/p1.jpg");
    pal.setBrush(QPalette::Window,QBrush(pixmap));
    setPalette(pal);

    loginui();                              //展示登录界面

    QTime syst1 = QTime::currentTime();     //可能用到随机数，设置随机数种子
    int tempt = syst0.msecsTo(syst1);
    qsrand(tempt);

    //connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::on_pushButton_clicked);    //自动连接，不用写
    //connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::on_pushButton_2_clicked);

    connect(ui->action_5,&QAction::triggered,this,&MainWindow::logout);        //登出
    connect(ui->action_2,&QAction::triggered,this,&MainWindow::getUserInfo);   //查看其他用户信息
    connect(ui->action_3,&QAction::triggered,this,[=](){                       //忘记密码
        QMessageBox::information(this,"Tips","Please send an email including your new password, to 'Glacier7777@foxmail.com' via the mailbox you set when registering.",QMessageBox::Ok);
    });
    connect(ui->action_4,&QAction::triggered,this,[=](){                       //游戏说明
        Dialog *dialog = new Dialog(this);
        dialog->show();
    });

    connect(ui->pushButton_3,&QPushButton::clicked,this,&MainWindow::choosefightPoke1);//升级赛，选择对战精灵
    connect(ui->pushButton_4,&QPushButton::clicked,this,&MainWindow::choosefightPoke2);//决斗赛，选择对战精灵

    client = new QTcpSocket(this);
    connect(client, &QTcpSocket::readyRead, this, &MainWindow::getServerMsg);  //收到信息

}
void MainWindow::getServerMsg()
{
    auto ret = client->read(BUF_LENGTH);
    QString msg = QString::fromLocal8Bit(ret);
    switch (state)            //REGISTER状态不在该页面进行信息的收发和处理，所以不处理
    {
        case LOGIN:           //LOGIN
        {
            client->disconnectFromHost();           //把之前连的7500端口给断了，使用返回的端口号
            int port = msg.toInt();
            if (port == 0)  //返回分配的端口值为0，说明遇到错误
            {
                QMessageBox::information(this, "warning", "error!", QMessageBox::Ok);
            }
            else
            {
                PORT = port;
                client->connectToHost(QHostAddress("127.0.0.1"), port);
                QMessageBox::information(this, "Tips", "登录成功!", QMessageBox::Ok);
                state = NORMAL;     //登录成功，进入正常状态
                thisname = curname; //当前用户名更新
                fightui();
            }
            break;
        }
        case USERLIST:                   //"USERLIST" (因为涉及到发送查询小精灵的消息，因此user List界面的实现放在此处而不是userlist.cpp)
        {
            QStringList users = msg.split('\n');         //不同用户之间用'\n'分割

            ul = new UserList(this);                     //创建用户信息页面
            ul->usertable->setRowCount(users.size()-1);  //设置table行数, 记得-1！！！（因为最后一个用户信息后面也有一个'\n'）
            for (int i = 0; i < users.size() - 1; i++)   //填入所有信息
            {
                QStringList detail = users[i].split(' ');
                //具体信息用' '分割: detail[0]=userid, detail[1]=username, detail[2]=y/n(在线与否)
                //detail[3]=victGame, detail[4]=allGame, detail[5]=M1, detail[6]=M2
                for (int j = 0; j < 3; j++)
                    ul->usertable->setItem(i, j, new QStandardItem(detail[j]));
                double rate = 0;
                if (detail[4].toInt() != 0)
                    rate = detail[3].toDouble()/detail[4].toDouble();
                QString vr = QString::number(rate, 'f', 2);
                vr += " (" + detail[3] + "/" + detail[4] + ")";
                ul->usertable->setItem(i, 3, new QStandardItem(vr));
                QStringList judgeMedal={"None","Bronze","Silver","Gold"};
                ul->usertable->setItem(i, 4, new QStandardItem(judgeMedal[detail[5].toInt()]));
                ul->usertable->setItem(i, 5, new QStandardItem(judgeMedal[detail[6].toInt()]));

                QPushButton *btn = new QPushButton(ul);
                ul->ui->tableView->setIndexWidget(ul->usertable->index(i, 6),btn);  //放入相应框内
                btn->setText("Check");                                              //设置按钮名称
                btn->setProperty("uid",detail[0].toInt());                          //记录参数:用户id
                connect(btn,&QPushButton::clicked,this,[=](){                       //连接起查询小精灵详细信息的函数
                    QPushButton *btn = (QPushButton *)sender();
                    QString str = "getPokemonList ";
                    str += btn->property("uid").toString();
                    str += '\n';
                    if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)         //担心发送出现错误，还是给个提示
                        QMessageBox::warning(this, "error", "server error!");
                    else
                    {
                        state = POKELIST;
                        curuid = btn->property("uid").toInt();                      //记录当前用户id，为了给小精灵界面命名
                    }
                });
            }
            ul->show();
            break;
        }
        case POKELIST:                      //"POKELIST"
        {
            PokemonList *pl = new PokemonList(curuid, msg, this);               //创建小精灵详细信息表
            pl->exec();                    //显示小精灵界面
            state = NORMAL;                //回到主界面
            break;
        }
        case GETMYPOKE:                    //选择我方出战精灵
        {
            allMyPoke = msg;               //这里顺便把所有小精灵信息记录下来，方便选择战败送出的精灵
            smpDialog *smp = new smpDialog(msg, this);
            connect(smp,SIGNAL(sendStr(QString)),this,SLOT(getStr(QString)));
            smp->exec();
            break;
        }
        default:
            break;
    }
}
void MainWindow::getLoseStr(QString pokeInfo)    //决斗赛失败，我方将要送出的精灵信息
{
    QStringList myPoke = pokeInfo.split(' ');
    QStringList detail = retnow.split(' ');
    detail[2] = myPoke[0];                      //将我方出战精灵id改成待删除的精灵id
    retnow = "";
    for (int i = 0;i < detail.size() - 1; i++)
    {
        retnow += detail[i] + " ";
    }
    retnow += detail[detail.size() - 1];         //重新打包战斗结果
    QString str = "battleResult " + retnow + '\n';
    if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)      //担心发送出现错误，还是给个提示
        QMessageBox::warning(this, "error", "server error!");
    else
        QMessageBox::information(this, "tips", "结果已发送到服务器端！\n以下是战斗详情！",QMessageBox::Ok);
    state = NORMAL;
}
void MainWindow::getStr(QString pokeInfo)    //我方出战精灵信息
{
    myPoke = pokeInfo;
    fffDialog *fff = new fffDialog(enemyPoke, myPoke, this);
    connect(fff,SIGNAL(sendRet(QString)),this,SLOT(getRet(QString)));
    fff->fight();
    fff->exec();
}

void MainWindow::getRet(QString ret)          //收到结果
{
    retnow = ret;
    if (ret[0] == "1" && ret[2] == "0")       //如果决斗赛输了，则要选择送出的精灵
    {
        QStringList det = allMyPoke.split('\n');
        QString msg = allMyPoke;
        if (det.size() > 4)                   //精灵数量不够3个，全列入可选名单；大于3个，随机挑3个
        {
            msg = "";
            int tem[3] = {rand() % (det.size() - 1)};   //选3个不重复的随机数
            while (tem[1] == tem[0])
                tem[1] = rand() % (det.size() - 1);
            while (tem[2] == tem[0] || tem[2] == tem[1])
                tem[2] = rand() % (det.size() - 1);

            for (int i = 0; i < 3; i++)
                msg += det[tem[i]] + '\n';
            msg += det[det.size() - 1];
        }
        smpDialog *smdp = new smpDialog(msg, this);
        smdp->setWindowTitle("战斗失败！请选择您要送出的Pokemon！");
        connect(smdp,SIGNAL(sendStr(QString)),this,SLOT(getLoseStr(QString)));
        QMessageBox::information(this, "tips", "模拟战斗结束！战斗失败！请选择您要送出的Pokemon！",QMessageBox::Ok);
        smdp->exec();
        return;
    }

    state = NORMAL;
    QString str = "battleResult " + ret + '\n';                  //战斗胜利就直接发送
    if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)      //担心发送出现错误，还是给个提示
        QMessageBox::warning(this, "error", "server error!");
    else
        QMessageBox::information(this, "tips", "模拟战斗结束！结果已发送到服务器端！\n以下是战斗详情！",QMessageBox::Ok);
}

void MainWindow::getPokeStr(QString vsPoke)   //收到选择的敌方精灵信息
{
    enemyPoke = vsPoke;
    state = GETMYPOKE;                        //得到敌方精灵信息后，获取我方精灵信息
    QString str = "nameToPoke " + thisname + '\n';
    if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)     //担心发送出现错误，还是给个提示
        QMessageBox::information(this, "error", "server error!",QMessageBox::Ok);
}

void MainWindow::fightui()                  //选择战斗比赛界面
{
    ui->label->close();
    ui->label_2->close();
    ui->lineEdit->close();
    ui->lineEdit_2->close();
    ui->pushButton->close();
    ui->pushButton_2->close();

    ui->pushButton_3->show();
    ui->pushButton_4->show();
}
void MainWindow::loginui()                  //登录界面
{
    ui->pushButton_3->close();
    ui->pushButton_4->close();

    ui->label->show();
    ui->label_2->show();
    ui->lineEdit->show();
    ui->lineEdit_2->show();
    ui->pushButton->show();
    ui->pushButton_2->show();
}

bool MainWindow::isValidStr(QString str)	//用户名和密码都只能有大小写字母、数字、下划线、句点、@号,3~25位
{
    if (str.isEmpty()) return false;
    if (str.length() < 3 || str.length() > 25) return false;
    for (auto c : str)
    {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '@'))
            return false;
    }
    return true;
}

void MainWindow::choosefightPoke1()
{
    cfDialog *cf = new cfDialog(this,false);
    connect(cf,SIGNAL(sendPokeStr(QString)),this,SLOT(getPokeStr(QString)));
    cf->exec();
}

void MainWindow::choosefightPoke2()
{
    cfDialog *cf = new cfDialog(this,true);
    connect(cf,SIGNAL(sendPokeStr(QString)),this,SLOT(getPokeStr(QString)));
    cf->exec();
}

void MainWindow::logout()                   //"登出"
{
    if ((state != LOGIN) && (state != REGISTER))
    {
        client->write("logout\n", BUF_LENGTH);
        client->disconnectFromHost();
        state = LOGIN;                      //回到待登录的状态
        loginui();
        QMessageBox::information(this, "tips", "登出成功!", QMessageBox::Ok);
    }
   else
    {
        QMessageBox::information(this,"Tips","登出失败！请确认已登录且回到主界面！",QMessageBox::Ok);
    }
}

void MainWindow::on_pushButton_clicked()    //"登录"按钮
{
    if (state != LOGIN)
    {
        QMessageBox::information(this,"Tips","登录失败！请确认尚未登录且回到主界面！",QMessageBox::Ok);
        return;
    }
    QString uname = ui->lineEdit->text();
    QString pwd = ui->lineEdit_2->text();
    curname = uname;
    if (!isValidStr(uname) || !isValidStr(pwd))
    {
        QMessageBox::information(this,"Tips","不合规范的用户名或密码! Check again. Only 'A'~'Z' && 'a'~'z' && '0'~'9' && '_' && '.' && '@'",QMessageBox::Ok);
    }
    else                                    //输入没有格式问题
    {
        client->connectToHost(QHostAddress("127.0.0.1"), 7500);
        QString str = "login ";
        str += uname;
        str += ' ';
        str += pwd;
        str += '\n';
        if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)     //担心发送出现错误，还是给个提示
            QMessageBox::warning(this, "error", "server error!");
    }
}
void MainWindow::on_pushButton_2_clicked()  //"注册"按钮
{
    if (state != LOGIN)
    {
        QMessageBox::information(this,"Tips","注册失败！请确认尚未登录且回到主界面！",QMessageBox::Ok);
        return;
    }
    rdialog = new RDialog(this);
    rdialog->exec();
    state = LOGIN;                          //注册完成后回到主界面
}
void MainWindow::getUserInfo()              //点击"查看用户信息"
{
    if ((state == LOGIN) || (state == REGISTER))
    {
        QMessageBox::information(this,"Tips","查看用户信息失败！请确认已登录且回到主界面！",QMessageBox::Ok);
        return;
    }
    state = USERLIST;
    QString str = "getUserList\n";
    if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)
    {
        QMessageBox::information(this,"warning","服务器错误！",QMessageBox::Ok);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}
