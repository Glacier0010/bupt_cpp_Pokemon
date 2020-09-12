#include "pokemonlist.h"
#include "ui_pokemonlist.h"
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QPalette>
#include <QPixmap>
#include <QDebug>

PokemonList::PokemonList(const int& uid, const QString& msg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PokemonList)
{
    ui->setupUi(this);
    setWindowTitle("Pokemon List of User whose ID=" + QString::number(uid));
    resize(800,600);
    poketable = new QStandardItemModel(this);      //创建表格
    ui->tableView->setModel(poketable);            //将ui的table view和poketable连接起来
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    poketable->setColumnCount(5);
    poketable->setHorizontalHeaderItem(0, new QStandardItem("PokemonID"));
    poketable->setHorizontalHeaderItem(1, new QStandardItem("kind"));
    poketable->setHorizontalHeaderItem(2, new QStandardItem("name"));
    poketable->setHorizontalHeaderItem(3, new QStandardItem("level"));
    poketable->setHorizontalHeaderItem(4, new QStandardItem("pic"));

    auto pokemons = msg.split('\n');
    poketable->setRowCount(pokemons.size()-1);
    for (int i = 0; i < pokemons.size() - 1; i++)
    {
       QStringList detail = pokemons[i].split(' '); // detail[0] is id, detail[1] is kind, detail[2] is level
       for (int j = 0; j < 2; j++)                                    //填id和kind
           poketable->setItem(i, j, new QStandardItem(detail[j]));
       QString tempk = detail[1];
       poketable->setItem(i, 2, new QStandardItem(nameDict.at(tempk.toInt())));//填名字
       poketable->setItem(i, 3, new QStandardItem(detail[2]));                 //填level

       QPushButton *btn = new QPushButton(this);
       ui->tableView->setIndexWidget(poketable->index(i, 4),btn);     //放入相应框内
       btn->setText("Check");                                         //设置按钮名称
       btn->setProperty("kind",detail[1]);                            //记录参数:小精灵的name
       connect(btn,&QPushButton::clicked,this,[=](){                  //连接起查询小精灵图片的函数
           QString filename = "./photo/" + btn->property("kind").toString() + ".jpg";
           //qDebug() << filename;

           QDialog *picdlg = new QDialog(this);
           picdlg->setAutoFillBackground(true);           //设置为背景图片（这样方便）
           picdlg->resize(300,300);
           QPalette pal;
           QPixmap pixmap(filename);
           pal.setBrush(QPalette::Window,QBrush(pixmap));
           picdlg->setPalette(pal);
           picdlg->exec();                                 //展示图片
       });
    }
}

PokemonList::~PokemonList()
{
    delete ui;
}
