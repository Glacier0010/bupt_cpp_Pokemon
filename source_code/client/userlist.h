#ifndef USERLIST_H
#define USERLIST_H

#include <QWidget>
#include <QDialog>
#include <QString>
#include <QTableView>
#include <QStandardItemModel>
#include "pokemonlist.h"
namespace Ui {
class UserList;
}

class UserList : public QDialog
{
    Q_OBJECT

public:
    QStandardItemModel *usertable;
    explicit UserList(QWidget *parent = 0);
    ~UserList();
    Ui::UserList *ui;

};

#endif // USERLIST_H
