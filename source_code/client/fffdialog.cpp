#include "fffdialog.h"
#include "ui_fffdialog.h"
#include <QPixmap>
#include <QDebug>

fffDialog::fffDialog(QString &vspoke, QString &mypoke, QWidget *parent) : QDialog(parent), ui(new Ui::fffDialog)
{
    ui->setupUi(this);
    setWindowTitle("战斗详情");
    QStringList temp = vspoke.split(" ");   //拆解敌方信息
    if (temp[0] == "y")
        battleGame = true;
    QString t = temp.at(1);
    vspokekind = t.toInt();
    t = temp.at(2);
    vspokelv = t.toInt();

    temp = mypoke.split(" ");               //拆解我方信息
    t = temp.at(0);
    mypokeid = t.toInt();
    t = temp.at(1);
    mypokekind = t.toInt();
    t = temp.at(2);
    mypokelv = t.toInt();

    setPhoto();                             //设置对战精灵图片

}
void fffDialog::setPhoto()
{
    QString filename = "./photo/" + QString::number(mypokekind) + ".jpg";
    QPixmap pic1(filename);
    filename = "./photo/" + QString::number(vspokekind) + ".jpg";
    QPixmap pic2(filename);
    ui->label->setPixmap(pic1);
    ui->label_2->setPixmap(pic2);
}

void fffDialog::fight()
{
    QTime t1 = QTime::currentTime();
    int tempi = time0.msecsTo(t1);
    qsrand(tempi);
    int myhp = setNum(1, mypokekind, mypokelv), vshp = setNum(1, vspokekind, vspokelv);        //我方血量和对方血量
    int myatk = setNum(0, mypokekind, mypokelv), vsatk = setNum(0, vspokekind, vspokelv);      //我方攻击值和对方攻击值
    int mydfc = setNum(2, mypokekind, mypokelv), vsdfc = setNum(2, vspokekind, vspokelv);      //我方防御值和对方防御值
    int myatkint = setNum(3, mypokekind, mypokelv), vsatkint = setNum(3, vspokekind, vspokelv);//我方攻击间隔和对方攻击间隔
    int mynextt = myatkint, vsnextt = vsatkint, myhpnew = myhp, vshpnew = vshp;
    QString bttext = "Game start!\n\n";

    int im;                             //一个重要参数
    while (myhpnew > 0 && vshpnew >0)
    {        
        ui->lineEdit->setText(QString::number(myhpnew) + "/" + QString::number(myhp));
        ui->lineEdit_2->setText(QString::number(vshpnew) + "/" + QString::number(vshp));
        if (((mynextt == vsnextt) && (rand() % 2 == 1)) || (mynextt < vsnextt))          //我方先出招  //如果两方攻击间隔相同，则以相同概率随机先开始攻击
        {
            int blood = 0;
            mynextt += myatkint;
            bttext += "我方【" + nameDict[mypokekind] + "】使出了【" + atkDict[mypokekind] + "】, ";
            //计算伤害值
            im = (mypokelv / 5 + 1) * 10;
            if (rand() % 100 >= im)                   //未达成miss
            {
                blood = (myatk - vsdfc) * 2 + vshp / 4 + (rand() % im - im / 2);//伤害值计算方法，可修改~~~~~~~~~~~~~~~~~~~~
                if (blood < 2) blood = 2;             //没有miss的条件下最起码也有2点伤害
                if (rand() % 100 < im)                //实现暴击
                {
                    bttext += "打出了暴击，威力上升50%，";
                    blood += blood / 2;               //暴击是1.5倍
                }
                bttext += "造成了" + QString::number(blood) + "点伤害.\n";
            }
            else                                      //对方miss
                bttext += "但对方miss了, 没有造成伤害.\n";
            vshpnew -= blood;
        }
        else                            //对方先出招
        {
            int blood = 0;
            vsnextt += vsatkint;
            bttext += "对方【" + nameDict[vspokekind] + "】使出了【" + atkDict[vspokekind] + "】, ";
            //计算伤害值
            im = (vspokelv / 5 + 1) * 10;
            if (rand() % 100 >= im)                   //未达成miss
            {
                blood = (vsatk - mydfc) * 2 + myhp / 4 + (rand() % im - im / 2);//~~~~~~~~~~~~~~~~~~~~~~~~~~~
                if (blood < 2) blood = 2;             //没有miss的条件下最起码也有2点伤害
                if (rand() % 100 < im)                //实现暴击
                    blood += blood / 2;               //暴击是1.5倍
                bttext += "造成了" + QString::number(blood) + "点伤害.\n";
            }
            else                                      //对方miss
                bttext += "但我方miss了, 没有造成伤害.\n";
            myhpnew -= blood;
        }
    }
    //有人挂了，结束战斗
    bttext += "\nGame over!!!\n";

    int exp = 0;                                      //计算经验值
    if (myhpnew <= 0)                                 //我方输了
    {
        ui->lineEdit->setText(QString::number(0) + "/" + QString::number(myhp));
        ui->lineEdit_2->setText(QString::number(vshpnew) + "/" + QString::number(vshp));
    }
    else                                              //我方胜利
    {
        victory = true;
        ui->lineEdit->setText(QString::number(myhpnew) + "/" + QString::number(myhp));
        ui->lineEdit_2->setText(QString::number(0) + "/" + QString::number(vshp));
        exp = vshp + myhpnew + (rand() % (2 * im)) - im / 2;
    }
    bttext += "结果为【";
    if (victory)
        bttext += "胜利】!\n获得经验：" + QString::number(exp) + ".\n";     //胜利后增加经验
    else
        bttext += "战败】！\n";
    if (battleGame)
    {
        if (victory)
            bttext += "您获得了1只" + QString::number(vspokelv) + "级的" + nameDict[vspokekind] + "!";
        else    //bttext += "您失去了1只" + QString::number(mypokelv) + "级的" + nameDict[mypokekind] + "!";
        {
        }
    }
    ui->textEdit->setText(bttext);

    QString ret = "0 ";                       //将战斗结果打包发给MainWindow
    if (battleGame)  ret ="1 ";
    if (victory)  ret += "1 ";
    else ret += "0 ";
    ret += QString::number(mypokeid) + " " + QString::number(vspokekind) + " " + QString::number(vspokelv) + " " + QString::number(exp);
    emit sendRet(ret);                        //发送ret

    //this->close();                          //战斗结束后自动关闭窗口
}


int fffDialog::setNum(int t, int kind, int lv)//t为待设置的属性（攻击值0/血量1/防御值2/攻击间隔3），kind为精灵种类，lv为精灵等级
{
    switch (t) {                            //t为待设置的属性（攻击值0/血量1/防御值2/攻击间隔3），kind为精灵种类（0~11），lv为精灵等级
    case 0:                                 //计算攻击值
        if (kind/3 == 0)                    //力量型
            return (12 + lv * 7);
        else
            return (10 + lv * 5);
    case 1:                                 //计算hp
        if (kind/3 == 1)                    //肉盾型
            return (104 + lv * 14);
        else
            return (100 + lv * 10);
    case 2:                                 //计算防御值
        if (kind/3 == 2)                    //防御型
            return (12 + lv * 7);
        else
            return (10 + lv * 5);
    case 3:                                 //计算攻击间隔
        if (kind/3 == 3)                    //敏捷型
            return (100 - lv * 3);
        else
            return (150 - lv * 2);
    }
    return 0;
}

fffDialog::~fffDialog()
{
    delete ui;
}
