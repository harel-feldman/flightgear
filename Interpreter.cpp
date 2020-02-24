//
// Created by roee on 17/12/2019.
//

#include "Interpreter.h"
#include <string>
#include "Expression.h"
#include <stack>
#include <queue>
#include <vector>
#include <map>
using namespace std;

BinaryOperator::BinaryOperator(Expression *left1, Expression *right1) {
    this->left = left1;
    this->right = right1;
}

BinaryOperator::~BinaryOperator() {
    delete (left);
    delete (right);
}

/*
double BinaryOperator::calculate() {
    //problem
    return this->left->calculate(), this->right->calculate();
}
*/

Plus::Plus(Expression *left1, Expression *right1) : BinaryOperator(left1, right1) {}

double Plus::calculate() {
    return (left->calculate() + right->calculate());
}

Minus::Minus(Expression *left1, Expression *right1) : BinaryOperator(left1, right1) {}

double Minus::calculate() {
    return (left->calculate() - right->calculate());
}

Div::Div(Expression *left1, Expression *right1) : BinaryOperator(left1, right1) {}

double Div::calculate() {
    if (right->calculate() == 0) {
        throw "Bad input";
    }

    return (left->calculate() / right->calculate());

}


Mul::Mul(Expression *left1, Expression *right1) : BinaryOperator(left1, right1) {}

double Mul::calculate() {
    return (left->calculate() * right->calculate());
}


UnaryOperator::UnaryOperator(Expression *exp1) {
    this->exp = exp1;

}

UnaryOperator::~UnaryOperator() {
    delete (exp);
}

/*
double UnaryOperator::calculate() {
    return exp->calculate();
} */

double UPlus::calculate() {
    return exp->calculate();
}

UPlus::UPlus(Expression *exp1) : UnaryOperator(exp1) {
}

UMinus::UMinus(Expression *exp1) : UnaryOperator(exp1) {
}

double UMinus::calculate() {
    return (-1) * exp->calculate();;
}


Value::Value(const double num) : value(num) {}

double Value::calculate() {
    return value;
}

double Variable::calculate() {
    return value;
}

Variable::Variable(string name1, double num1) : name(name1), value(num1) {}

Variable &Variable::operator++() {
    this->value += 1;
    return *this;
}

Variable &Variable::operator++(int) {
    this->value += 1;
    return *this;
}

Variable &Variable::operator--() {
    this->value -= 1;
    return *this;
}

Variable &Variable::operator--(int) {
    this->value -= 1;
    return *this;
}

Variable &Variable::operator+=(double num) {
    this->value += num;
    return *this;
}

Variable &Variable::operator-=(double num) {
    this->value -= num;
    return *this;
}

Expression *Interpreter::interpret(string str_exp) {
    queue<string> output;
    stack<char> operators;
    stack<Expression *> rpn;
    Expression *exp1;
    Expression *exp2;
    Expression *exp3;
    char last = ' ';
    string end = "";
    unsigned int i = 0, j = 0, k = 0;
    int flag = 0,counter=0;
    string num, tempo, v;

    while (i < str_exp.length()) {
        if(str_exp[i]==' ')
            i++;
        if (str_exp[i] > 47 && str_exp[i] < 58) {// number
            j = i;
            while ((j < str_exp.length()) && ((str_exp[j] > 47 && str_exp[j] < 58)
                                              || (str_exp[j] == 46))) {
                j++;
            }
            output.push(str_exp.substr(i, j - i));
            if ((last == 45 || last == 43)&&(i+1<str_exp.length() && (str_exp[i+1]!='/'&&str_exp[i+1]!='*'))) {
                v = operators.top();
                output.push(v);
                operators.pop();
            }
            i = j - 1;
        } else if (str_exp[i] == 43 || str_exp[i] == 45) {//+ -
            if (str_exp.length() == i + 1 || (str_exp[i + 1] > 41 && str_exp[i + 1] < 48)) {
                throw "illegal math expression";
            }
            if ((last == 40 || last == 32) &&
                str_exp[i] == 45) {// if the last char was "("  or the begining -its uminus operator
                operators.push('&');
            } else if ((last == 40 || last == 32) && str_exp[i] == 43) {
                operators.push('$');
            } else operators.push(str_exp[i]);
        } else if (str_exp[i] == '*' || str_exp[i] == '/') {// * /
            if (str_exp.length() == i + 1 || (str_exp[i + 1] > 41 && str_exp[i + 1] < 48)) {
                throw "illegal math expression";
            }
            if ((!operators.empty()) && (operators.top() == 42 || operators.top() == 47)) {
                string s = "";
                s = operators.top();
                output.push(s);
                operators.pop();
            }
            if (operators.empty() || operators.top() == 43 || operators.top() == 45 || operators.top() == 40||operators.top() == 38)
                operators.push(str_exp[i]);
        } else if (str_exp[i] == 40) {//  (
            operators.push(str_exp[i]);
            counter++;
        } else if (str_exp[i] == 41) {// )
            counter--;
            while ((!operators.empty()) && (operators.top() != 40)) {
                string sl = "";
                sl += operators.top();
                output.push(sl);
                operators.pop();
            }
            if (operators.empty()) {
                throw "illegal math expression";
            }
            else {
                operators.pop();
            }
        } else if ((str_exp[i] > 64 && str_exp[i] < 91) || (str_exp[i] > 96 && str_exp[i] < 123)) {
            k = i;
            while ((k < str_exp.length()) && ((str_exp[k] > 47 && str_exp[k] < 58) ||
                                              (str_exp[k] > 64 && str_exp[k] < 91) ||
                                              (str_exp[k] > 94 && str_exp[k] < 123))) {
                k++;
            }
            output.push(str_exp.substr(i, k - i));
            i = k - 1;
        }
        last = str_exp[i];
        i++;
    }
    while (!operators.empty()) {
        end = operators.top();
        output.push(end);
        operators.pop();
    }
    if(counter!=0){
        throw "illegal math expression";
    }

    // rpn- reading from output stack to expression stack
    while (!output.empty()) {
        string str = output.front();
        output.pop();
        if (str[0] < 58 && str[0] > 47) {//its number
            exp1 = new Value(stod(str));
            rpn.push(exp1);
        } else if (str[0] > 64) {//its var
            for (auto &variable : this->mapVariables) {
                if (variable.first == str) {
                    double d = variable.second->getValue();
                    exp2 = new Variable(str, d);
                    rpn.push(exp2);
                    flag = 1;
                }
            }
            if (flag == 0) throw "some variables have not been set";
        } else if (str[0] == 43) {//its +
            if (rpn.size() >= 2) {
                exp1 = rpn.top();
                rpn.pop();
                exp2 = rpn.top();
                rpn.pop();
                exp3 = new Plus(exp2, exp1);
                rpn.push(exp3);
            }
        } else if (str[0] == 45) {// its -
            if (rpn.size() >= 2) {
                exp1 = rpn.top();
                rpn.pop();
                exp2 = rpn.top();
                rpn.pop();
                exp3 = new Minus(exp2, exp1);
                rpn.push(exp3);
            }
        } else if (str[0] == 47) {// its /
            if (rpn.size() >= 2) {
                exp1 = rpn.top();
                rpn.pop();
                exp2 = rpn.top();
                rpn.pop();
                exp3 = new Div(exp2, exp1);
                rpn.push(exp3);
            }
        } else if (str[0] == 42) {// its *
            if (rpn.size() >= 2) {
                exp1 = rpn.top();
                rpn.pop();
                exp2 = rpn.top();
                rpn.pop();
                exp3 = new Mul(exp2, exp1);
                rpn.push(exp3);
            }
        } else if (str[0] == 36) {//its '$' uplus
            if (rpn.size() >= 1) {
                exp1 = rpn.top();
                rpn.pop();
                exp2 = new UPlus(exp1);
                rpn.push(exp2);
            }
        } else if (str[0] == 38) {//its '&' uminus
            if (rpn.size() >= 1) {
                exp1 = rpn.top();
                rpn.pop();
                exp2 = new UMinus(exp1);
                rpn.push(exp2);
            }
        }
    }
    return rpn.top();
}

Interpreter::Interpreter(map<string, Var *> &symbol_table) : mapVariables(symbol_table) {}
