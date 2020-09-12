#ifndef RDIALOG_H
#define RDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QTcpSocket>
#include <QHostAddress>
#include "parameter.h"

namespace Ui {
class RDialog;
}

class RDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RDialog(QWidget *parent = 0);
    ~RDialog();
    Ui::RDialog *ui;

private:
    QTcpSocket *client;
    bool isValidStr(QString str);	//用户名和密码都只能有大小写字母、数字、下划线、句点、@号,3~20位

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void readServerMsg();
};

#endif // RDIALOG_H
