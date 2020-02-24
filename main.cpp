#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cstring>
#include <algorithm>
#include "Command.h"
#include "Interpreter.h"

using namespace std;
//declarations
static map<string, double> symbol_table_from_simulator;
static map<string, Var *> symbol_table_from_text;
static queue<string> commandsQueue;
void lexer(queue<string> &arr, string line);

void buildMapCommands();

void clearSpaces(string &word);

void parser(vector<string> &commands, map<string, Command *> &commandsMap);

void buildMapSimulator();

static vector<string> commands;
static queue<string> lexer_queue;
static map<string, Command *> mapCommands;

int main() {

    buildMapCommands();
    buildMapSimulator();
    //change for main argv[1]
    std::ifstream file("fly.txt");
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            lexer(lexer_queue, line);
        }
        file.close();
    } else {
        cout << "Error in opening file" << endl;
    }
    //transfer data from the lexer queue to commands vector
    while (!lexer_queue.empty()){
        commands.push_back(lexer_queue.front());
        lexer_queue.pop();
    }

    parser(commands, mapCommands);
    commandsQueue.push("end_client");
    symbol_table_from_simulator.insert(make_pair("end_client",1));
    this_thread::sleep_for(chrono::seconds(7));
    return 0;
}

//
void parser(vector<string> &commands, map<string, Command *> &commandsMap) {
    vector<string> temp;
    int index = 0;
    Command *c;
    for (vector<string>::iterator it = commands.begin(); it != commands.end(); ++it) {
        while (*it != "EOL") {
            temp.push_back(*it);
            ++it;
        }
        //try
        if (temp.size() != 0) {
            string operation = commands[index];
            //מוסיף פה משהו ללולאות
            if (operation.compare("while") == 0 || operation.compare("if") == 0) {
                while (*it != "}") {
                    temp.push_back(*it);
                    ++it;
                }
            }
            auto iterator = commandsMap.find(operation);
            if (iterator != commandsMap.end()) {
                c = commandsMap.at(operation);
            } else {
                c = commandsMap.at("var");
            }
            // c== &commandsMap.at(commands[index]);
            if (c != NULL) {
                //what if we are waiting to the number?
                index += c->execute(temp);
            }
            //no memory allocating
            temp.resize(0);
        }
    }

}

void lexer(queue<string> &lexer_queue, string line) {

    replace(line.begin(), line.end(), '\t', ' ');
    clearSpaces(line);
    clearSpaces(line);
    clearSpaces(line);
    string word;
    string word2;
    clearSpaces(line);
    //save the first word from the line
    for (int i = 0; i < line.length(); i++) {
        while (line[i] != ' ' && line[i] != '(' && i < line.length()) {
            if (line[i] != '\t') {
                word = word + line[i];
            }
            i++;
        }
        break;
    }
    clearSpaces(word);
    if ((word.compare("Print") == 0) || (word.compare("Sleep") == 0)) {
        lexer_queue.push(word);

        word2 = line.substr(line.find('(') + 1);
        word2 = word2.substr(0, word2.length() - 1);

        lexer_queue.push(word2);
        lexer_queue.push("EOL");
        word2 = "";
    } else if ((word.compare("while") == 0) || (word.compare("if") == 0)) {
        lexer_queue.push(word);

        word2 = line.substr(word.length() + 1);
        word2 = word2.substr(0, word2.length() - 1);
        lexer_queue.push(word2);
        lexer_queue.push("{");
        lexer_queue.push("EOL");
        word2 = "";

    } else if (word.compare("var") == 0) {
        lexer_queue.push(word);
        word2 = line.substr(4);
        int token = line.find('=');
        string temp;
        if (line.find('=') == -1) {
            int x = word2.find('>');
            int y = word2.find('<');
            if (x > y) {
                temp = word2.substr(0, x - 2);
                lexer_queue.push(temp);
                lexer_queue.push("->");
                word2 = word2.substr(x + 1);
            } else {
                temp = word2.substr(0, y - 1);
                lexer_queue.push(temp);
                lexer_queue.push("<-");
                word2 = word2.substr(y + 1);
            }
            word2 = word2.substr(word2.find('(') + 1);
            word2 = word2.substr(1, word2.length() - 3);
            lexer_queue.push(word2);

        } else {
            string temp;
            temp = word2.substr(0, word2.find('='));
            clearSpaces(temp);
            lexer_queue.push(temp);
            lexer_queue.push("=");
            word2 = word2.substr(word2.find('=') +1);
            clearSpaces(word2);
            lexer_queue.push(word2);

        }
        lexer_queue.push("EOL");
    } else {
        if (line.find('=') == -1) {
            if (word.compare("}") != 0) {
                lexer_queue.push(word);
                word2 = line.substr(line.find('(') + 1);
                word2 = word2.substr(0, word2.length() - 1);
                clearSpaces(word2);
                lexer_queue.push(word2);
            } else {
                lexer_queue.push("}");
            }
        } else {
            string temp;
            temp = line.substr(0, line.find('='));
            clearSpaces(temp);
            lexer_queue.push(temp);
            lexer_queue.push("=");
            word2 = line.substr(line.find('=') + 1);
            clearSpaces(word2);
            lexer_queue.push(word2);
        }
        lexer_queue.push("EOL");

    }


}


void buildMapCommands() {
    mapCommands.insert(make_pair("openDataServer", new OpenServerCommand(symbol_table_from_simulator,symbol_table_from_text)));
    mapCommands.insert(make_pair("connectControlClient", new ConnectCommand(commandsQueue)));
    mapCommands.insert(make_pair("var", new DefineVarCommand(symbol_table_from_text, symbol_table_from_simulator,commandsQueue)));
    mapCommands.insert(make_pair("Print", new PrintCommand(symbol_table_from_text)));
    mapCommands.insert(make_pair("Sleep", new SleepCommand(symbol_table_from_text)));
    mapCommands.insert(make_pair("while", new ConditionCommand(symbol_table_from_text, symbol_table_from_simulator,mapCommands,
                                                               reinterpret_cast<void (*)(vector<std::string>,
                                                                                         map<std::string, struct Command *>)>(&parser))));
    mapCommands.insert(make_pair("if", new ConditionCommand(symbol_table_from_text, symbol_table_from_simulator,mapCommands,
                                                            reinterpret_cast<void (*)(vector<std::string>,
                                                                                      map<std::string, struct Command *>)>(&parser))));
}

void buildMapSimulator() {
    symbol_table_from_simulator["/instrumentation/airspeed-indicator/indicated-speed-kt"] = 0;
    symbol_table_from_simulator["/sim/time/warp"] = 0;
    symbol_table_from_simulator["/controls/switches/magnetos"] = 0;
    symbol_table_from_simulator["/instrumentation/heading-indicator/offset-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/altimeter/indicated-altitude-ft"] = 0;
    symbol_table_from_simulator["/instrumentation/altimeter/pressure-alt-ft"] = 0;
    symbol_table_from_simulator["/instrumentation/attitude-indicator/indicated-pitch-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/attitude-indicator/indicated-roll-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/attitude-indicator/internal-pitch-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/attitude-indicator/internal-roll-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/encoder/indicated-altitude-ft"] = 0;
    symbol_table_from_simulator["/instrumentation/encoder/pressure-alt-ft"] = 0;
    symbol_table_from_simulator["/instrumentation/gps/indicated-altitude-ft"] = 0;
    symbol_table_from_simulator["/instrumentation/gps/indicated-ground-speed-kt"] = 0;
    symbol_table_from_simulator["/instrumentation/gps/indicated-vertical-speed"] = 0;
    symbol_table_from_simulator["/instrumentation/heading-indicator/indicated-heading-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/magnetic-compass/indicated-heading-deg"] = 0;
    symbol_table_from_simulator["/instrumentation/slip-skid-ball/indicated-slip-skid"] = 0;
    symbol_table_from_simulator["/instrumentation/turn-indicator/indicated-turn-rate"] = 0;
    symbol_table_from_simulator["/instrumentation/vertical-speed-indicator/indicated-speed-fpm"] = 0;
    symbol_table_from_simulator["/controls/flight/aileron"] = 0;
    symbol_table_from_simulator["/controls/flight/elevator"] = 0;
    symbol_table_from_simulator["/controls/flight/rudder"] = 0;
    symbol_table_from_simulator["/controls/flight/flaps"] = 0;
    symbol_table_from_simulator["/controls/engines/engine/throttle"] = 0;
    symbol_table_from_simulator["/controls/engines/current-engine/throttle"] = 0;
    symbol_table_from_simulator["/controls/switches/master-avionics"] = 0;
    symbol_table_from_simulator["/controls/switches/starter"] = 0;
    symbol_table_from_simulator["/engines/active-engine/auto-start"] = 0;
    symbol_table_from_simulator["/controls/flight/speedbrake"] = 0;
    symbol_table_from_simulator["/sim/model/c172p/brake-parking"] = 0;
    symbol_table_from_simulator["/controls/engines/engine/primer"] = 0;
    symbol_table_from_simulator["/controls/engines/current-engine/mixture"] = 0;
    symbol_table_from_simulator["/controls/switches/master-bat"] = 0;
    symbol_table_from_simulator["/controls/switches/master-alt"] = 0;
    symbol_table_from_simulator["/engines/engine/rpm"] = 0;

}

void clearSpaces(string &word) {
    //remove space instead
    if (word[0] == ' ') {
        word = word.substr(1);
    }
    if (word[word.length() - 1] == ' ') {
        word = word.substr(0, word.length() - 1);
    }
}
