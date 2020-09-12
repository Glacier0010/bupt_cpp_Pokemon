#include "userlist.h"
#include "ui_userlist.h"
#include <QPushButton>
#include <QDebug>
UserList::UserList(QWidget *parent) : QDialog(parent), ui(new Ui::UserList)
{
    ui->setupUi(this);
    setWindowTitle("All users' detailed information");
    resize(970,830);
    usertable = new QStandardItemModel(this);      //创建表格
    ui->tableView->setModel(usertable);            //将ui的table view和usertable连接起来
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);//根据表头文字长度来确定列宽

    usertable->setColumnCount(7);
    usertable->setHorizontalHeaderItem(0, new QStandardItem("id"));
    usertable->setHorizontalHeaderItem(1, new QStandardItem("name"));
    usertable->setHorizontalHeaderItem(2, new QStandardItem("online or not"));
    usertable->setHorizontalHeaderItem(3, new QStandardItem("Victory Rate")); //用victGame&&allGame计算胜率
    usertable->setHorizontalHeaderItem(4, new QStandardItem("Pokemon number Medal"));
    usertable->setHorizontalHeaderItem(5, new QStandardItem("Super Pokemon Medal"));
    usertable->setHorizontalHeaderItem(6, new QStandardItem("Pokemon List"));

}

UserList::~UserList()
{
    delete ui;
}
