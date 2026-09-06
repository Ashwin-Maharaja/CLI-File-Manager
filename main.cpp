/*
========================================================
CLI FILE MANAGER
Tech: C++17, STL, std::filesystem
========================================================

FEATURES
- ls          -> List files
- cd          -> Change directory
- pwd         -> Print current directory
- mkdir       -> Create folder
- touch       -> Create file
- rm          -> Remove file/folder
- rename      -> Rename file/folder
- cp          -> Copy file
- help        -> Show commands
- exit        -> Quit program

Compile:
g++ -std=c++17 main.cpp -o filemanager

Run:
./filemanager
========================================================
*/

#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <functional>
#include <sstream>

namespace fs = std::filesystem;
using namespace std;

class CLIFileManager {
private:
    fs::path currentPath;

    unordered_map<string, function<void(vector<string>)>> commands;

public:
    CLIFileManager() {
        currentPath = fs::current_path();
        registerCommands();
    }

    void start() {
        cout << "===== CLI FILE MANAGER =====\n";
        cout << "Type 'help' to see commands.\n\n";

        string input;

        while (true) {
            cout << currentPath.string() << " > ";

            getline(cin, input);

            if (input.empty())
                continue;

            vector<string> tokens = parseInput(input);

            string cmd = tokens[0];

            if (cmd == "exit") {
                cout << "Exiting...\n";
                break;
            }

            if (commands.find(cmd) != commands.end()) {
                commands[cmd](tokens);
            } else {
                cout << "Unknown command.\n";
            }
        }
    }

private:

    vector<string> parseInput(const string& input) {
        vector<string> tokens;
        stringstream ss(input);
        string word;

        while (ss >> word) {
            tokens.push_back(word);
        }

        return tokens;
    }

    void registerCommands() {

        commands["help"] = [this](vector<string>) {
            showHelp();
        };

        commands["pwd"] = [this](vector<string>) {
            cout << currentPath << endl;
        };

        commands["ls"] = [this](vector<string>) {
            listFiles();
        };

        commands["cd"] = [this](vector<string> args) {
            if (args.size() < 2) {
                cout << "Usage: cd <directory>\n";
                return;
            }
            changeDirectory(args[1]);
        };

        commands["mkdir"] = [this](vector<string> args) {
            if (args.size() < 2) {
                cout << "Usage: mkdir <folder>\n";
                return;
            }
            createFolder(args[1]);
        };

        commands["touch"] = [this](vector<string> args) {
            if (args.size() < 2) {
                cout << "Usage: touch <file>\n";
                return;
            }
            createFile(args[1]);
        };

        commands["rm"] = [this](vector<string> args) {
            if (args.size() < 2) {
                cout << "Usage: rm <path>\n";
                return;
            }
            removeItem(args[1]);
        };

        commands["rename"] = [this](vector<string> args) {
            if (args.size() < 3) {
                cout << "Usage: rename <old> <new>\n";
                return;
            }
            renameItem(args[1], args[2]);
        };

        commands["cp"] = [this](vector<string> args) {
            if (args.size() < 3) {
                cout << "Usage: cp <source> <destination>\n";
                return;
            }
            copyFile(args[1], args[2]);
        };
    }

    void showHelp() {
        cout << "\nAvailable Commands:\n";
        cout << "----------------------------------\n";
        cout << "pwd                    -> Show current directory\n";
        cout << "ls                     -> List files/folders\n";
        cout << "cd <dir>               -> Change directory\n";
        cout << "mkdir <name>           -> Create folder\n";
        cout << "touch <file>           -> Create file\n";
        cout << "rm <path>              -> Delete file/folder\n";
        cout << "rename <old> <new>     -> Rename item\n";
        cout << "cp <src> <dest>        -> Copy file\n";
        cout << "help                   -> Show help\n";
        cout << "exit                   -> Quit\n";
        cout << "----------------------------------\n\n";
    }

    void listFiles() {
        try {
            for (const auto& entry : fs::directory_iterator(currentPath)) {

                if (fs::is_directory(entry.path()))
                    cout << "[DIR] ";
                else
                    cout << "[FILE] ";

                cout << entry.path().filename().string() << endl;
            }
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void changeDirectory(const string& dir) {
        fs::path newPath = currentPath / dir;

        if (fs::exists(newPath) && fs::is_directory(newPath)) {
            currentPath = fs::canonical(newPath);
        }
        else {
            cout << "Directory not found.\n";
        }
    }

    void createFolder(const string& folderName) {
        try {
            fs::create_directory(currentPath / folderName);
            cout << "Folder created.\n";
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void createFile(const string& fileName) {
        try {
            ofstream file(currentPath / fileName);

            if (file) {
                cout << "File created.\n";
            } else {
                cout << "Failed to create file.\n";
            }

            file.close();
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void removeItem(const string& name) {
        try {
            fs::remove_all(currentPath / name);
            cout << "Removed successfully.\n";
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void renameItem(const string& oldName, const string& newName) {
        try {
            fs::rename(currentPath / oldName,
                       currentPath / newName);

            cout << "Renamed successfully.\n";
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }

    void copyFile(const string& src, const string& dest) {
        try {
            fs::copy_file(
                currentPath / src,
                currentPath / dest,
                fs::copy_options::overwrite_existing
            );

            cout << "File copied successfully.\n";
        }
        catch (exception& e) {
            cout << "Error: " << e.what() << endl;
        }
    }
};

int main() {

    CLIFileManager manager;

    manager.start();

    return 0;
}
