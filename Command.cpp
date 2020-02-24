//
// Created by roee on 17/12/2019.
//
#include <sys/socket.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include "Command.h"
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include "Interpreter.h"
#include <thread>
#include <mutex>

double Var::getValue() {
    //if it is var which get value from simulator ,get the value from the simulator map and update.
    if (this->in1_out0 == 1) {
        double it = symbol_table_from_simulator.at(sim);
        this->value = it;
    }
    return this->value;
}

int Var::getDirection() { return this->in1_out0; }

string Var::getSim() { return this->sim; }

void Var::setValue(double new_val) {
    this->value = new_val;
    //update the sent field to be 0
    if (this->getDirection() == 0)
        this->sent = 0;
}

void Var::setSent() {
    this->sent = 1;
}

int Var::getSent() {
    return this->sent;
}

//the function gets data from simulator simultaneously and updates the maps.
void OpenServerCommand::readFromSimulator(int client_socket, map<string, double> &symbol_table_from_simulator) {
    std::this_thread::sleep_for(chrono::milliseconds((int) 2000));
    string s;
    mutex m;
    //array to save the parameters values from simulator
    float arr[36];
    string firstBuffer = "";
    while (symbol_table_from_simulator.size()<37) {
        char buffer[1024] = {0};
        read(client_socket, buffer, 1024);
        string secondBuffer = buffer;
        if (secondBuffer.compare("") == 0) break;
        firstBuffer = firstBuffer + secondBuffer;
        string firstToken = firstBuffer.substr(0, firstBuffer.find("\n"));
        string seconedToken = firstBuffer.substr(firstBuffer.find("\n") + 1, firstBuffer.length());
        m.lock();
        for (int i = 0; i < 36; i++) {
            int pos = firstToken.find(",");
            float val = stof(firstToken.substr(0, pos));
            arr[i] = val;
            firstToken = firstToken.substr(pos + 1);
        }
        //update the map
        symbol_table_from_simulator["/instrumentation/airspeed-indicator/indicated-speed-kt"] = arr[0];
        symbol_table_from_simulator["/sim/time/warp"] = arr[1];
        symbol_table_from_simulator["/controls/switches/magnetos"] = arr[2];
        symbol_table_from_simulator["/instrumentation/heading-indicator/offset-deg"] = arr[3];
        symbol_table_from_simulator["/instrumentation/altimeter/indicated-altitude-ft"] = arr[4];
        symbol_table_from_simulator["/instrumentation/altimeter/pressure-alt-ft"] = arr[5];
        symbol_table_from_simulator["/instrumentation/attitude-indicator/indicated-pitch-deg"] = arr[6];
        symbol_table_from_simulator["/instrumentation/attitude-indicator/indicated-roll-deg"] = arr[7];
        symbol_table_from_simulator["/instrumentation/attitude-indicator/internal-pitch-deg"] = arr[8];
        symbol_table_from_simulator["/instrumentation/attitude-indicator/internal-roll-deg"] = arr[9];
        symbol_table_from_simulator["/instrumentation/encoder/indicated-altitude-ft"] = arr[10];
        symbol_table_from_simulator["/instrumentation/encoder/pressure-alt-ft"] = arr[11];
        symbol_table_from_simulator["/instrumentation/gps/indicated-altitude-ft"] = arr[12];
        symbol_table_from_simulator["/instrumentation/gps/indicated-ground-speed-kt"] = arr[13];
        symbol_table_from_simulator["/instrumentation/gps/indicated-vertical-speed"] = arr[14];
        symbol_table_from_simulator["/instrumentation/heading-indicator/indicated-heading-deg"] = arr[15];
        symbol_table_from_simulator["/instrumentation/magnetic-compass/indicated-heading-deg"] = arr[16];
        symbol_table_from_simulator["/instrumentation/slip-skid-ball/indicated-slip-skid"] = arr[17];
        symbol_table_from_simulator["/instrumentation/turn-indicator/indicated-turn-rate"] = arr[18];
        symbol_table_from_simulator["/instrumentation/vertical-speed-indicator/indicated-speed-fpm"] = arr[19];
        symbol_table_from_simulator["/controls/flight/aileron"] = arr[20];
        symbol_table_from_simulator["/controls/flight/elevator"] = arr[21];
        symbol_table_from_simulator["/controls/flight/rudder"] = arr[22];
        symbol_table_from_simulator["/controls/flight/flaps"] = arr[23];
        symbol_table_from_simulator["/controls/engines/engine/throttle"] = arr[24];
        symbol_table_from_simulator["/controls/engines/current-engine/throttle"] = arr[25];
        symbol_table_from_simulator["/controls/switches/master-avionics"] = arr[26];
        symbol_table_from_simulator["/controls/switches/starter"] = arr[27];
        symbol_table_from_simulator["/engines/active-engine/auto-start"] = arr[28];
        symbol_table_from_simulator["/controls/flight/speedbrake"] = arr[29];
        symbol_table_from_simulator["/sim/model/c172p/brake-parking"] = arr[30];
        symbol_table_from_simulator["/controls/engines/engine/primer"] = arr[31];
        symbol_table_from_simulator["/controls/engines/current-engine/mixture"] = arr[32];
        symbol_table_from_simulator["/controls/switches/master-bat"] = arr[33];
        symbol_table_from_simulator["/controls/switches/master-alt"] = arr[34];
        symbol_table_from_simulator["/engines/engine/rpm"] = arr[35];
        m.unlock();
        firstBuffer = seconedToken;
    }
    close(client_socket);
}

//the function sends commands from commandsQueue to simulator simultaneously
void ConnectCommand::readFromText(int client_socket, queue<string> &commandsQueue) {
    mutex m;
    while (commandsQueue.front() != "end_client") {
        m.lock();
        while (!commandsQueue.empty()) {
            string temp = commandsQueue.front();
            int is_sent = send(client_socket, temp.c_str(), strlen(temp.c_str()), 0);
            commandsQueue.pop();
        }
        m.unlock();
    }
    close(client_socket);
}

int OpenServerCommand::execute(vector<string> vec) {
    Interpreter *interpreter = new Interpreter(symbol_table_from_text);
    Expression *e = interpreter->interpret(vec[1]);
    double p = e->calculate();
    int port = (int) p;
    stringstream str;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_socket;
    //create the sockaddr obj.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    //bind socket to IP address
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        std::cerr << "Could not bind the socket to an IP" << std::endl;
        return -2;
    }
    if (listen(server_fd, 5) == -1) { //can also set to SOMAXCON (max connections)
        std::cerr << "Error during listening command" << std::endl;
        return -3;
    } else {
        std::cout << "Server is now listening ..." << std::endl;
    }
    client_socket = accept(server_fd, (struct sockaddr *) &address,
                           (socklen_t *) &addrlen);
    if (client_socket == -1) {
        std::cerr << "Error accepting client" << std::endl;
        return -4;
    }
    std::cout << "Server is connected" << endl;
    //open thread of server function which get data from simulator simultaneously
    thread t1(readFromSimulator, client_socket, ref(symbol_table_from_simulator));
    t1.detach();
    //num of jumping
    return 3;
}


int ConnectCommand::execute(vector<string> vec) {
    string temp = vec[1];
    int pos = temp.find(",");
    string ip = temp.substr(1, pos - 2);
    map<string, Var *> empty_map;
    Interpreter *interpreter = new Interpreter(empty_map);
    const char *ip_c[ip.length()];
    Expression *e = interpreter->interpret(temp.substr(pos + 1));
    double p = e->calculate();
    strcpy((char *) ip_c, ip.c_str());
    int port = (int) p;
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        //error
        std::cerr << "Could not create a socket" << std::endl;
        return -1;
    }
    //create a sockaddr obj to hold address of server
    sockaddr_in address;
    address.sin_family = AF_INET;//IP4
    address.sin_addr.s_addr = inet_addr(reinterpret_cast<const char *>(ip_c));  //the localhost address
    address.sin_port = htons(port);

    // Requesting a connection with the server on local host with port 8081
    int is_connect = connect(client_socket, (struct sockaddr *) &address, sizeof(address));
    if (is_connect == -1) {
        std::cerr << "Could not connect to host server" << std::endl;
        return -2;
    } else {
        std::cout << "Client is now connected to server" << std::endl;
    }
    //open thread of client function which send to simulator commands from commandQueue simultaneously
    thread t2(readFromText, client_socket, ref(commandsQueue));
    t2.detach();
    return 3;
}

int DefineVarCommand::execute(vector<string> vec) {
    Interpreter *interpreter = new Interpreter(symbol_table_from_text);
    int dir = 1;
    string path = "";
    double value;
    bool flag = false;
    string name;
    mutex m;
    m.lock();
    // if it is new var it defines it and add to maps
    if (vec[0].compare("var") == 0) {
        name = vec[1];
        //there is a case og '=' - it is var the simulator does'nt know ,add it to the map.
        if (vec[2].compare("=") == 0) {
            flag = true;
            Expression *e = interpreter->interpret(vec[3]);
            Var *tempo = new Var(2, e->calculate(), "", symbol_table_from_simulator);
            this->symbol_table_from_text.insert(make_pair(vec[1], tempo));
            //it is definition, add it to the map.
        } else {
            if (vec[2].compare("->") == 0) {
                dir = 0;
            }
            path = vec[3];
            Var *t = new Var(dir, 0, path, symbol_table_from_simulator);
            this->symbol_table_from_text.insert(make_pair(name, t));
        }
        //gives values to var we already defined
    } else {
        name = vec[0];
        flag = true;
        Expression *e = interpreter->interpret(vec[2]);
        value = e->calculate();
        this->symbol_table_from_text[vec[0]]->setValue(value);
        path = symbol_table_from_text.at(name)->getSim();
    }
    //if the var need to be sent to the simulator add a messege to commandQueue
    if (symbol_table_from_text.at(name)->getDirection() == 0 && flag) {
        string mes = "set " + path.substr(1) + " " + to_string(value) + "\r\n";
        commandsQueue.push(mes);
    }
    m.unlock();
    return vec.size() + 1;
}

int PrintCommand::execute(vector<string> vec) {
    string printLine = vec[1];
    if (vec[1][0] != 34) {//34 is the char " in ascii
        Interpreter *i = new Interpreter(symbol_table_from_text);
        Expression *e = i->interpret(vec[1]);
        cout << e->calculate() << endl;
    } else {
        printLine = printLine.substr(1, printLine.length() - 2);
        cout << printLine << endl;
    }
    return 3;
}

int SleepCommand::execute(vector<string> vec) {
    Interpreter *interpreter = new Interpreter(symbol_table_from_text);
    Expression *e = interpreter->interpret(vec[1]);
    std::this_thread::sleep_for(chrono::milliseconds((int) e->calculate()));
    return 3;
}

int ConditionCommand::execute(vector<string> vec) {
    vector<string> vec1;
    Interpreter *interpreter = new Interpreter(symbol_table_from_text);
    int i = 0;
    double val = 0;
    char sign;
    int flag = 0;
    string temp = "";
    //constracts the first expresion
    while (vec[1][i] > 64 || vec[1][i] < 60) {
        if (vec[1][i] != ' ')
            temp += vec[1][i];
        i++;
    }
    string condition_var = temp;
    auto it = symbol_table_from_text.find(condition_var);
    Var *vari = it->second;
    //value of first var
    double value = vari->getValue();
    sign = vec[1][i];
    i++;
    //assign value to flag which will express the operator sign
    if (sign == '<')
        flag = 1;
    if (sign == '=')
        flag = 3;
    if (vec[1][i] == '=') {
        flag = flag + 2;
        i++;
    }
    temp = "";
    //constracts the second expression
    while (i < vec[1].length()) {
        if (vec[1][i] != ' ')
            temp += vec[1][i];
        i++;
    }
    Expression *e = interpreter->interpret(temp);
    //val-value of the second expression
    val = e->calculate();
    for (auto it = vec.begin() + 4; it != vec.end(); ++it) {
        vec1.push_back(*it);
    }
    if (vec[0].compare("while") == 0) {
        //the flag value expresses the operator in the condition
        while (((flag == 0) && (value > val)) || ((flag == 1) && (value < val)) || ((flag == 3) && (value <= val)) ||
               ((flag == 4) && (value >= val)) || ((flag == 5) && (value == val))) {
            //send all lines in the loop to parser function
            parser(vec1, mapCommand);
            //updates value of first expression
            value = symbol_table_from_text.find(condition_var)->second->getValue();
            e = interpreter->interpret(temp);
            //updates value of second expression
            val = e->calculate();
        }
    } else if (vec[0].compare("if") == 0) {
        //the flag value expresses the operator in the condition
        if (((flag == 0) && (value > val)) || ((flag == 1) && (value < val)) || ((flag == 3) && (value <= val)) ||
            ((flag == 4) && (value >= val)) || ((flag == 5) && (value == val))) {
            //send all lines in the condition body to parser function
            parser(vec1, mapCommand);
        }
    }
    return vec.size() + 2;

}