#ifndef FFFDIALOG_H
#define FFFDIALOG_H

#include <QDialog>
#include <QString>
#include <QTime>

#include "smpdialog.h"

namespace Ui {
class fffDialog;
}

class fffDialog : public QDialog
{
    Q_OBJECT

public:
    explicit fffDialog(QString &vspoke, QString &mypoke, QWidget *parent = 0);
    ~fffDialog();
    void fight();                   //模拟决斗过程，并显示在文本框

private:
    Ui::fffDialog *ui;

    const QStringList nameDict = { "Hitmonlee","Primeape","Machop","Chansey","Jigglypuff","Snorlax","Squirtle","Milotic","Slowbro","Pikachu","Charmander","Butterfree" };
    const QStringList atkDict = { "百万吨飞腿","近身战","爆裂拳","舍身一击","泰山压顶","沉重轰炸","火箭头槌","终极冲击","意念头锤","十万伏特","喷射火焰","破坏光线" };

    bool battleGame = false;                 //决斗赛与否
    int mypokeid = 0;
    int mypokekind = 0;
    int mypokelv = 1;
    int vspokekind = 0;
    int vspokelv = 1;
    QTime time0 = QTime::currentTime();

    bool victory = false;                   //本局胜负

    void setPhoto();                        //显示双方图片
    int setNum(int t, int kind, int lv);    //设置参数

signals:
    void sendRet(QString);                  //发送战斗结果

};

#endif // FFFDIALOG_H
