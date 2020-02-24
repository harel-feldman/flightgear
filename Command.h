//
// Created by harelfeldman on 12/15/19.
//
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include "Expression.h"
#include <queue>
#include <list>
#include <thread>
#include "Interpreter.h"

using namespace std;

#ifndef FLIGHTGEAR_COMMNAD_H
#define FLIGHTGEAR_COMMNAD_H

class Var {
private:
    int in1_out0;
    double value;
    string sim;
    int sent;
public:
    map<string, double> &symbol_table_from_simulator;

    Var(int direction, double val, string simu, map<string, double> &symbol_table) : symbol_table_from_simulator(
            symbol_table) {
        in1_out0 = direction;
        value = val;
        sim = std::move(simu);
        sent = 1;
    };

    int getDirection();

    double getValue();

    string getSim();

    void setValue(double new_val);

    void setSent();

    int getSent();
};

class Command {
protected:
   // Interpreter* interpreter;
public:
    virtual int execute(vector<string>) = 0;

    virtual ~Command() = default;
};


class OpenServerCommand : public Command {
public:
    map<string, double> &symbol_table_from_simulator;
    map<string, Var *> &symbol_table_from_text;
    OpenServerCommand(map<string, double> &symbol_table,map<string, Var *> &from_text) : symbol_table_from_simulator(symbol_table),
    symbol_table_from_text(from_text){};

    int execute(vector<string> vec) override;
   static void  readFromSimulator(int client_socket,map<string, double> &symbol_table_from_simulator);
};

class ConnectCommand : public Command {
public:

    //map<string, Var *> &symbol_table_from_text;
    queue<string> &commandsQueue;
    explicit ConnectCommand(queue<string>& qu) : commandsQueue(qu) {};

    int execute(vector<string> vec) override;

    static void  readFromText(int client_socket,queue<string>&commandsQueue);
};

class DefineVarCommand : public Command {
public:
    map<string, Var *>& symbol_table_from_text;
    map<string, double> &symbol_table_from_simulator;
    queue<string> &commandsQueue;

    DefineVarCommand(map<string, Var *> &symbol_table, map<string, double> &from_simulator,queue<string> &qu) :
    commandsQueue(qu),symbol_table_from_text(symbol_table),symbol_table_from_simulator(from_simulator) {};

    int execute(vector<string> vec) override;
};

class PrintCommand : public Command {
public:
    map<string, Var *> &symbol_table_from_text;

    explicit PrintCommand(map<string, Var *> &symbol_table) : symbol_table_from_text(symbol_table) {};

    int execute(vector<string> vec) override;
};

class SleepCommand : public Command {
public:
       map<string, Var *> &symbol_table_from_text;

    explicit SleepCommand(map<string, Var *> &symbol_table) : symbol_table_from_text(symbol_table) {};
    int execute(vector<string> vec) override;
};

class ConditionCommand : public Command {
public:

    map<string, Var *> &symbol_table_from_text;
    map<string, double> &symbol_table_from_simulator;
    map<string,Command*> &mapCommand;
    void (*parser)(vector<string>, map<string, Command *>);
//the constructor gets maps, queue and the parser function.
   ConditionCommand(map<string, Var *> &symbol_table, map<string, double> &from_simulator,map<string,Command*> &mapCommands
           ,void (* func)(vector<string>, map<string, Command *>)):
           symbol_table_from_text(symbol_table),
           symbol_table_from_simulator(from_simulator),mapCommand(mapCommands) {
           parser=func;
   };

    int execute(vector<string> vec) override;
};

#endif //FLIGHTGEAR_COMMNAD_H