#include <math.h>//��� ������� ��� �����������
#include <fstream>
#include <string>
#include <string_view>
#include <cctype>//��� isdigit


class Expression {//����������� ����� ���������
public:
	virtual Expression* derivative(std::string_view str) = 0;//������ �����������,��� str ��� ���������� �����������������
	virtual int eval(std::string_view str) = 0;//�����������
	virtual void print(std::ostream& out) = 0;//����� ��������� � ����
	virtual ~Expression() = 0;//����� ����������� ����������
};
Expression::~Expression() {};

class Number :public Expression
{
private:

	int num;

public:

	Number() : num(0){}

	Number(int n) : num(n) {}

	int eval(std::string_view str) override {
		return num;
	}

	Expression* derivative(std::string_view str) override
	{
		return new Number();//����������� �� ��������� - ����
	}

	void print(std::ostream& out) override
	{
		out << num;//��� ������ ����� � ��������� ������ �� ��������
	}
};

class Variable :public Expression
{
private:

	std::string var;

public:

	Variable(): var("x"){}

	Variable(std::string str): var(str){}

	Variable(Variable& other) {
		var = other.var;
	}

	int eval(std::string_view str) override {
		if (str.find(var) == -1)
			throw std::exception("�������� �����������");

		std::string s = var + " <- ";//���� ��� ������ ���������� � �����������
		if (str.find(s) != 0)//���� ���� ����������� �� �����
			s = " " + var + " <- ";
		
		size_t idx = str.find(s) + var.length() + 4;//������ ������� �������� ����������� ����������
		std::string asw;//������ - �������� ����������
		for (; str[idx] != ';' && idx < str.length(); idx++) {//���� �� ������ � ����� ������ ���...
			asw += str[idx];//...���� �� ������� ����������� ��������� ����������
		}
		return stoi(asw);
	}

	Expression* derivative(std::string_view str) override
	{
		if (var == str)
			return new Number(1);
		else
			return new Number();
	}

	void print(std::ostream& out) override
	{
		out << var;
	}
};

class Add :public Expression
{
private:

	Expression* m1, * m2;

public:

	Add(): m1(nullptr), m2(nullptr){}

	Add(Expression* L1, Expression* L2) : m1(L1), m2(L2) {}

	~Add() override {
		delete m1;
		delete m2;
	}
	
	int eval(std::string_view str) override {
		return (m1->eval(str) + m2->eval(str));
	}

	Expression* derivative(std::string_view str) override
	{
		return new Add(m1->derivative(str), m2->derivative(str));

	}

	void print(std::ostream& out) override {
		out << "(";
		m1->print(out);
		out << "+";
		m2->print(out);
		out << ")";
	}
};

class Sub :public Expression
{
private:

	Expression* m1, * m2;

public:

	Sub() : m1(nullptr), m2(nullptr) {}

	Sub(Expression* L1, Expression* L2) : m1(L1), m2(L2) {}

	~Sub() override {
		delete m1;
		delete m2;
	}

	int eval(std::string_view str) override {
		return (m1->eval(str) - m2->eval(str));
	}

	Expression* derivative(std::string_view str) override
	{
		return new Sub(m1->derivative(str), m2->derivative(str));

	};

	void print(std::ostream& out) override {
		out << "(";
		m1->print(out);
		out << "-";
		m2->print(out);
		out << ")";
	}
};

class Mul :public Expression
{
private:

	Expression* m1;
	Expression* m2;

public:

	Mul() : m1(nullptr), m2(nullptr) {}

	Mul(Expression* L1, Expression* L2) : m1(L1), m2(L2) {}

	~Mul() override {
		delete m1;
		delete m2;
	}

	int eval(std::string_view str) override {
		return (m1->eval(str) * m2->eval(str));
	}

	Expression* derivative(std::string_view str) override
	{
		return new Add(new Mul(m1->derivative(str), m2), new Mul(m1, m2->derivative(str)));

	};

	void print(std::ostream& out) override {
		out << "(";
		m1->print(out);
		out << "*";
		m2->print(out);
		out << ")";
	}
};

class Div :public Expression
{
private:

	Expression* m1;
	Expression* m2;

public:

	Div() : m1(nullptr), m2(nullptr) {}

	Div(Expression* L1, Expression* L2) : m1(L1), m2(L2) {}

	~Div() override {
		delete m1;
		delete m2;
	}

	int eval(std::string_view str) override {
		if (m2->eval(str) == 0)
			throw std::exception("������� �� ����");
		return (int)(m1->eval(str) / m2->eval(str));
	}

	Expression* derivative(std::string_view str) override
	{
		return new Div(new Sub(new Mul(m1->derivative(str), m2), new Mul(m1, m2->derivative(str))), new Mul(m2, m2));
	};

	void print(std::ostream& out) override {
		out << "(";
		m1->print(out);
		out << ")" << "/" << "(";
		m2->print(out);
		out << ")";
	}
};

int found(std::string_view str) {//���� ������ ����� ��������
	size_t a = 0, b = 0, i = 0;
	if (str[0] != '(') {
		if (isdigit(str[0]))
			return 0;//���� �������� �����
		return 1;//���� �������� ����������
	}
	for (; a - b != 1 || str[i] == '('; i++) {//������� ����� � ������, ��� ���������� ���� ��������
		if (str[i] == '(')
			a++;
		if (str[i] == ')')
			b++;
	}//���� �������������, ����� �� ����� �������� �� ������� ������� ������ (������, ���������� ��� �����)
	for (; str[i] != ')'; i++) {
		if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/')
			return i;
	}//���� ���� ������ ������� ��������� ���� ��������
}

Expression* scan(std::string_view str) {
	size_t idx = found(str);
	std::string s1;
	std::string s2;
	if (str.length() > 1){
		s1 = str.substr(1, idx - 1);
		s2 = str.substr(idx + 1, str.length() - idx - 2);
	}

	//���������� �������� ����� ��������� �� �����
	if(idx == 1)
		return new Variable(std::string(str));

	if (idx == 0)
		return new Number(stoi(std::string(str)));
	
	if (str[idx] == '+')
		return new Add(scan(std::string_view(s1)), scan(std::string_view(s2)));

	if (str[idx] == '-')
		return new Sub(scan(std::string_view(s1)), scan(std::string_view(s2)));

	if (str[idx] == '*')
		return new Mul(scan(std::string_view(s1)), scan(std::string_view(s2)));

	if (str[idx] == '/')
		return new Div(scan(std::string_view(s1)), scan(std::string_view(s2)));
}


int main()
{
	std::ifstream fin{ "input.txt" };
	std::ofstream fout{ "output.txt" };

	std::string str;
	fin >> str;
	Expression* i = scan(std::string_view(str));

	Expression* di = i->derivative("x");
	di->print(fout);
	fout << std::endl;

	try {
		//�����������
	}
	catch (const std::exception &error) {
		fout << "������: " << error.what();
	}


	delete i, di;

	return 0;
}