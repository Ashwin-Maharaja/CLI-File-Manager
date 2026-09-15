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

LEVEL 1
- Colored terminal output
- Colored directories
- Colored files
- Colored success/error messages

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
#include <vector>

namespace fs = std::filesystem;
using namespace std;


// ==================== TERMINAL COLORS ====================

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"


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

        cout << CYAN
             << "===== CLI FILE MANAGER ====="
             << RESET << "\n";

        cout << "Type "
             << YELLOW << "'help'"
             << RESET
             << " to see commands.\n\n";

        string input;

        while (true) {

            cout << BLUE
                 << currentPath.string()
                 << RESET
                 << " > ";

            getline(cin, input);

            if (input.empty())
                continue;


            vector<string> tokens = parseInput(input);

            string cmd = tokens[0];


            if (cmd == "exit") {

                cout << YELLOW
                     << "Exiting..."
                     << RESET << "\n";

                break;
            }


            if (commands.find(cmd) != commands.end()) {

                commands[cmd](tokens);

            } else {

                cout << RED
                     << "Unknown command."
                     << RESET << "\n";
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

    // ==================== COMMAND REGISTRY ====================

    void registerCommands() {


        commands["help"] = [this](vector<string>) {

            showHelp();
        };


        commands["pwd"] = [this](vector<string>) {

            cout << CYAN
                 << currentPath
                 << RESET << endl;
        };


        commands["ls"] = [this](vector<string>) {

            listFiles();
        };


        commands["cd"] = [this](vector<string> args) {

            if (args.size() < 2) {

                cout << YELLOW
                     << "Usage: cd <directory>"
                     << RESET << "\n";

                return;
            }

            changeDirectory(args[1]);
        };


        commands["mkdir"] = [this](vector<string> args) {

            if (args.size() < 2) {

                cout << YELLOW
                     << "Usage: mkdir <folder>"
                     << RESET << "\n";

                return;
            }

            createFolder(args[1]);
        };


        commands["touch"] = [this](vector<string> args) {

            if (args.size() < 2) {

                cout << YELLOW
                     << "Usage: touch <file>"
                     << RESET << "\n";

                return;
            }

            createFile(args[1]);
        };


        commands["rm"] = [this](vector<string> args) {

            if (args.size() < 2) {

                cout << YELLOW
                     << "Usage: rm <path>"
                     << RESET << "\n";

                return;
            }

            removeItem(args[1]);
        };


        commands["rename"] = [this](vector<string> args) {

            if (args.size() < 3) {

                cout << YELLOW
                     << "Usage: rename <old> <new>"
                     << RESET << "\n";

                return;
            }

            renameItem(args[1], args[2]);
        };


        commands["cp"] = [this](vector<string> args) {

            if (args.size() < 3) {

                cout << YELLOW
                     << "Usage: cp <source> <destination>"
                     << RESET << "\n";

                return;
            }

            copyFile(args[1], args[2]);
        };
    }

    // ==================== HELP ====================

    void showHelp() {

        cout << "\n"
             << CYAN
             << "Available Commands:"
             << RESET << "\n";

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


    // ==================== LIST FILES ====================

    void listFiles() {

        try {

            for (const auto& entry :
                 fs::directory_iterator(currentPath)) {


                if (fs::is_directory(entry.path())) {

                    cout << BLUE
                         << "[DIR]  "
                         << entry.path().filename().string()
                         << RESET
                         << endl;

                }

                else {

                    cout << GREEN
                         << "[FILE] "
                         << entry.path().filename().string()
                         << RESET
                         << endl;
                }
            }
        }


        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET
                 << endl;
        }
    }

    // ==================== CHANGE DIRECTORY ====================

    void changeDirectory(const string& dir) {

        fs::path newPath = currentPath / dir;


        if (fs::exists(newPath) &&
            fs::is_directory(newPath)) {

            currentPath = fs::canonical(newPath);
        }

        else {

            cout << RED
                 << "Directory not found."
                 << RESET
                 << "\n";
        }
    }

    // ==================== CREATE FOLDER ====================

    void createFolder(const string& folderName) {

        try {

            fs::create_directory(
                currentPath / folderName
            );

            cout << GREEN
                 << "Folder created."
                 << RESET
                 << "\n";
        }


        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET
                 << endl;
        }
    }

    // ==================== CREATE FILE ====================

    void createFile(const string& fileName) {

        try {

            ofstream file(
                currentPath / fileName
            );


            if (file) {

                cout << GREEN
                     << "File created."
                     << RESET
                     << "\n";
            }

            else {

                cout << RED
                     << "Failed to create file."
                     << RESET
                     << "\n";
            }


            file.close();
        }


        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET
                 << endl;
        }
    }

    // ==================== REMOVE FILE/FOLDER ====================

    void removeItem(const string& name) {

        try {

            fs::remove_all(
                currentPath / name
            );

            cout << GREEN
                 << "Removed successfully."
                 << RESET
                 << "\n";
        }


        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET
                 << endl;
        }
    }

    // ==================== RENAME ====================

    void renameItem(
        const string& oldName,
        const string& newName
    ) {

        try {

            fs::rename(
                currentPath / oldName,
                currentPath / newName
            );

            cout << GREEN
                 << "Renamed successfully."
                 << RESET
                 << "\n";
        }


        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET
                 << endl;
        }
    }

    // ==================== COPY FILE ====================

    void copyFile(
        const string& src,
        const string& dest
    ) {

        try {

            fs::copy_file(
                currentPath / src,
                currentPath / dest,
                fs::copy_options::overwrite_existing
            );

            cout << GREEN
                 << "File copied successfully."
                 << RESET
                 << "\n";
        }


        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET
                 << endl;
        }
    }
};

int main() {
    CLIFileManager manager;
    manager.start();
    return 0;
}