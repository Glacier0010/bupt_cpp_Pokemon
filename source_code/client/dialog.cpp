#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle("游戏说明");
    resize(500,700);
    ui->textEdit->clear();
    ui->textEdit->setPlainText("###############2020.8.6##############\nThis is 'My Pokemon' by Glacier!\n这个粗糙的游戏还有很多不足，但是包含了all my love && tears...\n如果您有任何建设性意见，欢迎您的反馈，十分感谢！\n\n以下是关于该游戏的一些基本信息。\n##########Basic Information###########\n这是一款借鉴了Pokemon的精灵对战类游戏。该游戏中一共分为4种精灵，分别是【力量型】、【肉盾型】、【防御型】和【敏捷型】。每种精灵下分为3个亚种。12种精灵的详细信息参见【用户手册】。\n用户通过【注册账号】（用户名不可重复）、【登录账户】、【选择比赛类型&&出战精灵】、【对战】以体验游戏。比赛分为2种：升级赛和决斗赛。注册账号后，系统随机分配3只1级精灵。通过比赛，用户可以为精灵升级、获得新精灵，达到一定成绩后可获得徽章。\nEnjoy yourself！\n\n以下是常见问题及回答。\n############FAQ######################\n【Q1】游戏里的比赛总是和系统派出的精灵决斗吗？可不可以和其他玩家联网决斗？\n【A】是的（因此这款游戏不像对战类而是更像种田类）。由于技术所限，暂时无法与其它玩家互动决斗。\n\n【Q2】升级赛和决斗赛有何区别？\n【A】升级赛胜利时，参战的小精灵会获得经验，积累一定经验就会升级（变得更强）。而决斗赛胜利时，不仅参战小精灵获得经验，而且用户还会获得对战的小精灵（等级不变）；但如果失败，则会失去1只小精灵(系统在您的精灵表中任意选择3只供您选择，若少于3只则全部可选择)。\n\n【Q3】如果我一直输掉决斗赛，小精灵都输光了怎么办？\n【A】那么在下一次对战前，系统会自动随机为您分配一只1级精灵。\n\n【Q4】小精灵满级是多少级？\n【A】15级。\n\n【Q5】徽章有哪些？需要满足什么条件？\n【A】徽章分为【宠物个数徽章】和【高级宠物徽章】。分别体现了用户拥有的小精灵个数和满级精灵个数的多少。分为【gold】【silver】【bronze】。获得这些徽章的具体条件参见【用户手册】。所有注册用户获得徽章的情况可在游戏主窗口的上方【查看】-【所有用户信息】处了解到。\n\n【Q6】注册时为什么要留邮箱？\n【A】本意是如果您忘记密码，可通过注册邮箱发邮件给我以找回密码。我方借邮箱判断是否为您本人操作。（不过应该用不上:）\n\n");

}

Dialog::~Dialog()
{
    delete ui;
}