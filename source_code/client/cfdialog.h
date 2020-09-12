#ifndef CFDIALOG_H
#define CFDIALOG_H
#include <QTime>
#include <QDialog>

namespace Ui {
class cfDialog;
}

class cfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit cfDialog(QWidget *parent = 0, bool getPokemon = false);    //getPokemon=true时为决斗赛，否则为升级赛
    ~cfDialog();
    Ui::cfDialog *ui;

private:
    bool battleGame = false;
    QTime t0 = QTime::currentTime();
    const QStringList nameDict = { "Hitmonlee","Primeape","Machop","Chansey","Jigglypuff","Snorlax","Squirtle","Milotic","Slowbro","Pikachu","Charmander","Butterfree" };
    int generateRand(int minn, int maxx);

private slots:
    void chooseFight();         //选择对战精灵
    void updateui();            //刷新界面

signals:
    void sendPokeStr(QString);  //发送选择的对战精灵信息
};

#endif // CFDIALOG_H
