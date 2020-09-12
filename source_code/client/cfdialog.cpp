#include "cfdialog.h"
#include "ui_cfdialog.h"

#include <QString>
#include <QStringList>
#include <QPushButton>

cfDialog::cfDialog(QWidget *parent, bool getPokemon):   //getPokemon=true时为决斗赛，否则为升级赛
    QDialog(parent),  ui(new Ui::cfDialog)
{
    ui->setupUi(this);
    if (getPokemon)
        setWindowTitle("选择决斗赛对战精灵！");
    else
        setWindowTitle("选择升级赛对战精灵！");
    battleGame = getPokemon;
    ui->pushButton->resize(145,60);
    ui->pushButton_2->resize(145,60);
    ui->pushButton_3->resize(145,60);
    ui->pushButton_4->resize(145,60);
    ui->pushButton_5->resize(145,60);
    ui->pushButton_6->resize(145,60);

    QTime t1 = QTime::currentTime();            //设置随机数种子
    int tempi = t0.msecsTo(t1);
    qsrand(tempi);

    updateui();

    connect(ui->pushButton,&QPushButton::clicked,this,&cfDialog::chooseFight);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&cfDialog::chooseFight);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&cfDialog::chooseFight);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&cfDialog::chooseFight);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&cfDialog::chooseFight);
    connect(ui->pushButton_6,&QPushButton::clicked,this,&cfDialog::chooseFight);
    connect(ui->pushButton_7,&QPushButton::clicked,this,&cfDialog::updateui);
}

void cfDialog::updateui()                       //更新可选择的对战精灵列表
{
    QStringList text;
    int kind[6],lv[6];
    for (int i = 0;i < 6;i++)
    {
        kind[i] = generateRand(0,11);          //随机生成一个精灵
        lv[i] = generateRand(1,15);            //随机生成一个等级
        QString temp = nameDict[kind[i]] + '\n' + "level: " + QString::number(lv[i]);
        text.append(temp);
    }
    ui->pushButton->setText(text.at(0));
    ui->pushButton_2->setText(text.at(1));
    ui->pushButton_3->setText(text.at(2));
    ui->pushButton_4->setText(text.at(3));
    ui->pushButton_5->setText(text.at(4));
    ui->pushButton_6->setText(text.at(5));

    ui->pushButton->setProperty("kind",kind[0]);
    ui->pushButton->setProperty("lv",lv[0]);

    ui->pushButton_2->setProperty("kind",kind[1]);
    ui->pushButton_2->setProperty("lv",lv[1]);

    ui->pushButton_3->setProperty("kind",kind[2]);
    ui->pushButton_3->setProperty("lv",lv[2]);

    ui->pushButton_4->setProperty("kind",kind[3]);
    ui->pushButton_4->setProperty("lv",lv[3]);

    ui->pushButton_5->setProperty("kind",kind[4]);
    ui->pushButton_5->setProperty("lv",lv[4]);

    ui->pushButton_6->setProperty("kind",kind[5]);
    ui->pushButton_6->setProperty("lv",lv[5]);
}

int cfDialog::generateRand(int minn, int maxx)      //产生随机数
{
    int randN = qrand();
    return (randN % (maxx - minn + 1) + minn);
}

void cfDialog::chooseFight()                        //选择对战精灵
{
    QPushButton *btn = (QPushButton *)sender();
    QString vsPoke = "n ";
    if (battleGame) vsPoke = "y ";                   //如果是决斗赛，就设置为“y”
    vsPoke += btn->property("kind").toString() + ' '+ btn->property("lv").toString();
    emit sendPokeStr(vsPoke);
    this->close();                                   //关闭该界面
}

cfDialog::~cfDialog()
{
    delete ui;
}
