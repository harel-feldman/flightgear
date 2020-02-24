//
// Created by roee on 17/12/2019.
//



#ifndef UPDATEDSIM_INTERPRETER_H
#define UPDATEDSIM_INTERPRETER_H
#include <map>
#include <string>
#include <list>
#include "queue"
#include "Expression.h"
#include "Command.h"
using namespace std;
/*
class Expression {
public:
    virtual double calculate() = 0;
    virtual ~Expression();

};
 */
class Interpreter {

private:
    map<string,Var* > &mapVariables;


public:
    Interpreter(map<string, Var *> &mapVariables);

    Expression *interpret(string str);
};
class BinaryOperator : public Expression {
protected:
    Expression *left;
    Expression *right;
public:
    BinaryOperator(Expression *left, Expression *right);

    ~BinaryOperator();


};

class Minus : public BinaryOperator {
public:
    Minus(Expression *left, Expression *right);

    virtual double calculate();
};

class Mul : public BinaryOperator {
public:
    Mul(Expression *left, Expression *right);

    virtual double calculate();
};

class Div : public BinaryOperator {
public:
    Div(Expression *left, Expression *right);

    virtual double calculate();
};

class Plus : public BinaryOperator {
public:
    Plus(Expression *left, Expression *right);

    virtual double calculate();

};


class UnaryOperator : public Expression {
protected:
    Expression *exp;
public:
    UnaryOperator(Expression *exp);

    ~UnaryOperator();
};


class UPlus : public UnaryOperator {

public:
    UPlus(Expression *exp);


    virtual double calculate();
};

class UMinus : public UnaryOperator {

public:
    explicit UMinus(Expression *exp);

    double calculate() override;
};


class Value : public Expression {
private:
    double value;
public:
    Value(const double num);

    //virtual?
    virtual double calculate();
};

class Variable : public Expression {
private:
    string name;
    double value;
public:
    Variable(string name, double value);

    double calculate();

    Variable &operator++();

    Variable &operator++(int);

    Variable &operator--();

    Variable &operator--(int);

    Variable &operator+=(double num);

    Variable &operator-=(double num);
    // ~Variable();

};

#endif //UPDATEDSIM_INTERPRETER_H
