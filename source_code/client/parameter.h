#ifndef PARAMETER_H
#define PARAMETER_H

const int BUF_LENGTH=1024;

//以下为客户端的几种状态
#define NORMAL 0    //正常登录后
#define LOGIN 1     //尝试登录
#define REGISTER 2  //尝试注册
#define USERLIST 3  //查看用户信息表
#define POKELIST 4  //查看精灵信息表
#define GETMYPOKE 5 //获取我方精灵表
#define LOSE 6      //对战失败，选择送出的精灵

#endif // PARAMETER_H
