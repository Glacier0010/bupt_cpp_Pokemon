#ifndef SMPDIALOG_H
#define SMPDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class smpDialog;
}

class smpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit smpDialog(QString &msg, QWidget *parent = 0);
    ~smpDialog();

private:
    Ui::smpDialog *ui;
    QStandardItemModel *poketable;
    const QStringList nameDict = { "Hitmonlee","Primeape","Machop","Chansey","Jigglypuff","Snorlax","Squirtle","Milotic","Slowbro","Pikachu","Charmander","Butterfree" };

signals:
    void sendStr(QString);  //发送选择的对战精灵信息
};

#endif // SMPDIALOG_H
