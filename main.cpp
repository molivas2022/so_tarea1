#include <iostream>
#include <string>
#include <vector>
#include <sstream>

bool DEBUG_PARSER = true;

using namespace std;

struct Command {
    string name;
    vector<string> args;
};

/*
TODO: cuando se ingresan dos | | seguidos
TODO ?: cuando se ingresa | seguido del string vacio
*/
vector<Command> parser(string inputstr) {
    vector<Command> output;

    /* separar por espacios */
    vector<string> words;
    {
    istringstream iss(inputstr);
    string word;
    while (iss >> word) words.push_back(word);
    }

    /* leer cada comando y sus argumentos */
    {
    Command cmd;
    int mode = 0;
    for (auto word: words) {
        if (mode == 0) {
            cmd.name = word;
            mode = 1;
        }
        else if (mode == 1) {
            if (word == "|") {
                output.push_back(cmd);
                cmd = Command();
                mode = 0;
                continue;
            }
            cmd.args.push_back(word);
        }
    }
    output.push_back(cmd);
    }

    /* debug */
    if (DEBUG_PARSER) {
        for (auto cmd: output) {
            cout << "command:\n\t" << cmd.name << endl;
            cout << "args:" << endl;
            for (auto arg: cmd.args) {
                cout << "\t" << arg << endl;
            }
            cout << endl;
        }
    }

    return output;
}

int main() {
    while (true) {
        cout << "$ ";
        string input;
        getline(cin, input);
        parser(input);
    }
}