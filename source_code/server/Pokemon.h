#pragma once
#include <iostream>

using namespace std;

#define STRENGTH_TYPE 0				//������
#define SHIELD_TYPE   1				//�����
#define DEFENSE_TYPE  2				//������
#define SWIFT_TYPE    3				//������

//����ΪС���������ֵ����ʽ�ֵ䣨��kind���ࣩ
const char nameDict[12][11] = { "Hitmonlee","Primeape","Machop","Chansey","Jigglypuff","Snorlax","Squirtle","Milotic","Slowbro","Pikachu","Charmander","Butterfree" };
const char atkDict[12][11] = { "����ַ���","����ս","����ȭ","����һ��","̩ɽѹ��","���غ�ը","���ͷ�","�ռ����","����ͷ��","ʮ�����","�������","�ƻ�����" };

class PokemonBase {						//���о���Ļ���			
protected:
	int type;							//����
	int kind;							//���������(0~11)
	char name[11];						//����
	int level;							//�ȼ�
	int exp;							//����ֵ
	int attack;							//������
	int defence;						//������
	int hp;								//����ֵ
	int atk_interval;					//�������
public:
	PokemonBase();
	PokemonBase(int nkind);				//����kindֵ����ʼ����������
	~PokemonBase() {};
	void Levelup();						//����
	void Output();						//���������Ϣ��������Ŀ1����
	virtual void atk_method() = 0;		//�����أ�ÿ�־��鹥��������ͬ		
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
	strcpy_s(name, "Default");		//������Ϊ��Ĭ�ϡ�
}
PokemonBase::PokemonBase(int nkind) : type(nkind/3), kind(nkind), level(1), exp(0), attack(10), defence(10), hp(100), atk_interval(150) {
	strcpy_s(name, nameDict[nkind]);	//����kind�����������
	switch (type) {
	case 0:							//�����ͣ�����ֵ��
		attack = 12;
		break;
	case 1:							//����ͣ�����ֵ��
		hp = 104;
		break;
	case 2:							//�����ͣ�����ֵ��
		defence = 12;
		break;
	case 3:							//�����ͣ����������
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
	case 0:							//������
		attack += 2;
		break;
	case 1:							//�����
		hp += 4;
		break;
	case 2:							//������
		defence += 2;
		break;
	case 3:							//������
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
