#include "rdialog.h"
#include "ui_rdialog.h"

RDialog::RDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RDialog)
{
    ui->setupUi(this);
    setWindowTitle("Register");
    resize(400,250);
    //connect(ui->buttonBox,&QDialogButtonBox::clicked,this,&RDialog::on_buttonBox_clicked);//这是自动连接的，不注释就会运行两遍

    // about network
    client = new QTcpSocket(this);
    connect(client, &QTcpSocket::readyRead, this, &RDialog::readServerMsg);
}
bool RDialog::isValidStr(QString str)	//用户名、密码和邮箱都只能有大小写字母、数字、下划线、句点、@号,3~25位
{
    if (str.isEmpty()) return false;
    if (str.length()<3 || str.length()>25) return false;
    for (auto c : str)
    {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '@'))
            return false;
    }
    return true;
}
void RDialog::readServerMsg()
{
    auto ret = client->read(BUF_LENGTH);
    client->disconnectFromHost();
    if (QString(ret) == "Accepted.\n")
    {
        QMessageBox::information(this, "Tips", "注册成功!", QMessageBox::Ok);
        accept();                   // back to mainwindow
    }
    else
    {
        QMessageBox::information(this, "Warning", QString(ret), QMessageBox::Ok);
    }
}
RDialog::~RDialog()
{
    delete ui;
}
void RDialog::on_buttonBox_clicked(QAbstractButton *button)
{

    if (ui->buttonBox->button(QDialogButtonBox::Ok)  == button)     //确认按的是"ok"按钮
    {
        QString uname = ui->lineEdit->text();
        QString pwd = ui->lineEdit_2->text();
        QString pwd2 = ui->lineEdit_3->text();
        QString mbox = ui->lineEdit_4->text();
        if (!isValidStr(uname) || !isValidStr(pwd))     //用户名或密码格式有误
        {
            QMessageBox::information(this,"warning","不合规范的用户名或密码！请再次检查。注意：3~20个字符。只含有'A'~'Z' && 'a'~'z' && '0'~'9' && '_' && '.' && '@'",QMessageBox::Ok);
        }
        else
        {
            if (pwd.compare(pwd,pwd2)!=0)               //确认密码有误
            {
                QMessageBox::information(this,"warning","确认密码有误！",QMessageBox::Ok);
            }
            else
                if (!isValidStr(mbox))                  //邮箱格式有误
                {
                    QMessageBox::information(this,"warning","邮箱格式错误！",QMessageBox::Ok);
                }
                else                                    //一切顺利
                {
                    client->connectToHost(QHostAddress("127.0.0.1"), 7500);
                    QString str = "register ";
                    str += uname;
                    str += " ";
                    str += pwd;
                    str += " ";
                    str += mbox;
                    str += '\n';
                    ui->buttonBox->setDisabled(true);
                    if (client->write(str.toLocal8Bit(), BUF_LENGTH) == -1)
                    {
                        QMessageBox::information(this,"warning","服务器错误！",QMessageBox::Ok);
                        ui->buttonBox->setDisabled(false);
                    }
                }
        }
    }
}
