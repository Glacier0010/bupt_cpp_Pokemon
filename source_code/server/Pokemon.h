#pragma once
#include <iostream>

using namespace std;

#define STRENGTH_TYPE 0				//力量型
#define SHIELD_TYPE   1				//肉盾型
#define DEFENSE_TYPE  2				//防御型
#define SWIFT_TYPE    3				//敏捷型

//以下为小精灵名称字典和招式字典（按kind分类）
const char nameDict[12][11] = { "Hitmonlee","Primeape","Machop","Chansey","Jigglypuff","Snorlax","Squirtle","Milotic","Slowbro","Pikachu","Charmander","Butterfree" };
const char atkDict[12][11] = { "百万吨飞腿","近身战","爆裂拳","舍身一击","泰山压顶","沉重轰炸","火箭头槌","终极冲击","意念头锤","十万伏特","喷射火焰","破坏光线" };

class PokemonBase {						//所有精灵的基类			
protected:
	int type;							//种类
	int kind;							//具体的类型(0~11)
	char name[11];						//名字
	int level;							//等级
	int exp;							//经验值
	int attack;							//攻击力
	int defence;						//防御力
	int hp;								//生命值
	int atk_interval;					//攻击间隔
public:
	PokemonBase();
	PokemonBase(int nkind);				//根据kind值，初始化所有属性
	~PokemonBase() {};
	void Levelup();						//升级
	void Output();						//输出属性信息，用于题目1测试
	virtual void atk_method() = 0;		//待重载，每种精灵攻击方法不同		
	int getType() const { return type; };
	int getKind() const { return kind; };
	int getLv() const { return level; };
	int getExp() const { return exp; };
	int getAtk() const { return attack; };
	int getDfc() const { return defence; };
	int getHp() const { return hp; };
	int getATKINT() const { return atk_interval; };
};
PokemonBase::PokemonBase() :type(0), kind(0), level(1), exp(0), attack(1), defence(1), hp(100), atk_interval(40) {
	strcpy_s(name, "Default");		//名字设为“默认”
}
PokemonBase::PokemonBase(int nkind) : type(nkind/3), kind(nkind), level(1), exp(0), attack(10), defence(10), hp(100), atk_interval(150) {
	strcpy_s(name, nameDict[nkind]);	//根据kind序号设置名字
	switch (type) {
	case 0:							//力量型：攻击值高
		attack = 12;
		break;
	case 1:							//肉盾型：生命值高
		hp = 104;
		break;
	case 2:							//防御型：防御值高
		defence = 12;
		break;
	case 3:							//敏捷型：攻击间隔低
		atk_interval = 100;
	}
}
void PokemonBase::Levelup() {
	if (level == 15) return;
	level++;
	attack += 5;
	defence += 5;
	hp += 10;
	atk_interval -= 2;
	switch (type) {
	case 0:							//力量型
		attack += 2;
		break;
	case 1:							//肉盾型
		hp += 4;
		break;
	case 2:							//防御型
		defence += 2;
		break;
	case 3:							//敏捷型
		atk_interval--;
	}
}
void PokemonBase::Output() {
	cout << name << " |(level) " << level << " |(exp) " << exp << " |(attack) " << attack << " |(defence) " <<
		defence << " |(hp) " << hp << " |(atk_int) " << atk_interval << endl;
}

class Pokemon :public PokemonBase {
public:
	Pokemon();
	Pokemon(int nkind);
	~Pokemon() {};
	void atk_method();
};
Pokemon::Pokemon() :PokemonBase() {};
Pokemon::Pokemon(int nkind) :PokemonBase(nkind) {};
void Pokemon::atk_method() {
	cout << name << ": " << atkDict[kind] << "!!!" << endl;
}
