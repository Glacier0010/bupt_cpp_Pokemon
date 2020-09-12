#include "smpdialog.h"
#include "ui_smpdialog.h"

#include <QPushButton>
#include <QString>
smpDialog::smpDialog(QString &msg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::smpDialog)
{
    ui->setupUi(this);
    setWindowTitle("Select one of your Pokemons!");
    resize(800,600);
    poketable = new QStandardItemModel(this);      //创建表格
    ui->tableView->setModel(poketable);            //将ui的table view和poketable连接起来
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->resize(800,600);
    poketable->setColumnCount(5);
    poketable->setHorizontalHeaderItem(0, new QStandardItem("PokemonID"));
    poketable->setHorizontalHeaderItem(1, new QStandardItem("kind"));
    poketable->setHorizontalHeaderItem(2, new QStandardItem("name"));
    poketable->setHorizontalHeaderItem(3, new QStandardItem("level"));
    poketable->setHorizontalHeaderItem(4, new QStandardItem("select"));

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

       QPushButton *btn = new QPushButton(this);                      //设置按钮
       ui->tableView->setIndexWidget(poketable->index(i, 4),btn);     //放入相应框内
       btn->setText("this one!");                                     //设置按钮名称
       QString t = detail[0] + ' ' + detail[1] + ' '+ detail[2];      //连接要显示和发送的信息文本
       btn->setProperty("info",t);                                    //记录参数:小精灵的id+kind+lv
       connect(btn,&QPushButton::clicked,this,[=](){                  //连接“点击”行为和以下操作
           QPushButton *btn = (QPushButton *)sender();
           this->hide();
           emit sendStr(btn->property("info").toString());            //发送信息！
       });
    }
}

smpDialog::~smpDialog()
{
    delete ui;
}
