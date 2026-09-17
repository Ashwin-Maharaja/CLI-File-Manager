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
- search      -> Search files
- tree        -> Display directory tree
- reindex     -> Rebuild file index
- help        -> Show commands
- exit        -> Quit program

LEVEL 1
- Colored terminal output
- Colored directories
- Colored files
- Colored success/error messages

LEVEL 2
- Recursive directory tree
- UTF-8 tree visualization

LEVEL 3
- Recursive file search

LEVEL 4
- Command piping
- ls | search
- tree | search

LEVEL 5
- Command parser
- Quoted arguments
- Quote validation
- Argument validation
- Command registry

LEVEL 6
- File indexing
- Indexed search
- Background indexing using std::thread
- Thread-safe index using std::mutex
- Automatic index updates after file operations
- Manual index rebuilding with reindex

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
#include <windows.h>
#include <thread>
#include <mutex>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;


// ==================== TERMINAL COLORS ====================

#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"


struct Command {
    string name;
    vector<string> arguments;
};

struct CommandInfo {
    int minArguments;
    int maxArguments;
};

struct FileEntry {
    string name;
    fs::path path;
    bool isDirectory;
};

class CLIFileManager {

private:

    fs::path currentPath;

    unordered_map<string, function<void(vector<string>)>> commands;
    unordered_map<string, CommandInfo> commandInfo;
    vector<FileEntry> fileIndex;
    mutex indexMutex;
    thread indexingThread;

public:

    CLIFileManager() {

        currentPath = fs::current_path();

        registerCommands();

        indexingThread = thread(&CLIFileManager::buildIndex,this);
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

            if (input.find('|') != string::npos) {

                size_t pipePos = input.find('|');

                string firstCommand = input.substr(0, pipePos);
                string secondCommand = input.substr(pipePos + 1);

                Command first = parseCommand(firstCommand);
                Command second = parseCommand(secondCommand);

                if (first.name.empty() || second.name.empty()) {
                    cout << YELLOW
                         << "Invalid pipe syntax."
                         << RESET << endl;
                    continue;
                }

                string pipedOutput;

                if (first.name == "ls") {
                    pipedOutput = listFiles();
                }
                else if (first.name == "tree") {
                    pipedOutput = showTreeForSearch();
                }

                if (second.name == "search" && second.arguments.size() >= 1) {
                    cout << searchFiles(second.arguments[0], pipedOutput);
                }

                continue;
            }

            Command command = parseCommand(input);

            if (command.name.empty())
                continue;

            string cmd = command.name;

            if (cmd == "exit") {

                cout << YELLOW
                     << "Exiting..."
                     << RESET << "\n";

                break;
            }

            if (commands.find(cmd) == commands.end()) {

                cout << RED
                     << "Unknown command."
                     << RESET << "\n";

                continue;
            }

            if (!validateCommand(command))
                continue;

            commands[cmd](command.arguments);
        }
    }

    ~CLIFileManager() {
        if (indexingThread.joinable()) {
            indexingThread.join();
        }
    }

private:

    vector<string> parseInput(const string& input, bool& validQuotes) {

        vector<string> tokens;
        string token;
        bool insideQuotes = false;

        for (char c : input) {

            if (c == '"') {
                insideQuotes = !insideQuotes;
            }

            else if (c == ' ' && !insideQuotes) {

                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            }

            else {
                token += c;
            }
        }

        if (!token.empty()) {
            tokens.push_back(token);
        }

        validQuotes = !insideQuotes;

        return tokens;
    }

    Command parseCommand(const string& input) {

        bool validQuotes;

        vector<string> tokens = parseInput(input, validQuotes);

        Command command;

        if (!validQuotes) {
            cout << YELLOW
                 << "Error: Unclosed quotation mark."
                 << RESET << endl;

            return command;
        }

        if (tokens.empty())
            return command;

        command.name = tokens[0];

        for (size_t i = 1; i < tokens.size(); i++) {
            command.arguments.push_back(tokens[i]);
        }

        return command;
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

            string output = listFiles();

            stringstream ss(output);
            string line;

            while (getline(ss, line)) {

                if (line.rfind("[DIR]", 0) == 0)
                    cout << BLUE << line << RESET << endl;

                else if (line.rfind("[FILE]", 0) == 0)
                    cout << GREEN << line << RESET << endl;

                else
                    cout << line << endl;
            }
        };


        commands["tree"] = [this](vector<string>) {
        
            showTree();
        };


        commands["search"] = [this](vector<string> args) {
            if (args.size() < 1) {
                cout << YELLOW << "Usage: search <name>\n" << RESET;
                return;
            }

            cout << searchFiles(args[0]);
        };


        commands["cd"] = [this](vector<string> args) {

            if (args.size() < 1) {

                cout << YELLOW
                     << "Usage: cd <directory>"
                     << RESET << "\n";

                return;
            }

            changeDirectory(args[0]);
        };


        commands["mkdir"] = [this](vector<string> args) {

            if (args.size() < 1) {

                cout << YELLOW
                     << "Usage: mkdir <folder>"
                     << RESET << "\n";

                return;
            }

            createFolder(args[0]);
        };


        commands["touch"] = [this](vector<string> args) {

            if (args.size() < 1) {

                cout << YELLOW
                     << "Usage: touch <file>"
                     << RESET << "\n";

                return;
            }

            createFile(args[0]);
        };


        commands["rm"] = [this](vector<string> args) {

            if (args.size() < 1) {

                cout << YELLOW
                     << "Usage: rm <path>"
                     << RESET << "\n";

                return;
            }

            removeItem(args[0]);
        };


        commands["rename"] = [this](vector<string> args) {

            if (args.size() < 2) {

                cout << YELLOW
                     << "Usage: rename <old> <new>"
                     << RESET << "\n";

                return;
            }

            renameItem(args[0], args[1]);
        };


        commands["cp"] = [this](vector<string> args) {

            if (args.size() < 2) {

                cout << YELLOW
                     << "Usage: cp <source> <destination>"
                     << RESET << "\n";

                return;
            }

            copyFile(args[0], args[1]);
        };

        commands["reindex"] = [this](vector<string>) {
        
            buildIndex();

            cout << GREEN
                 << "Index rebuilt successfully."
                 << RESET
                 << "\n";
        };


        commandInfo["help"]   = {0, 0};
        commandInfo["pwd"]    = {0, 0};
        commandInfo["ls"]     = {0, 0};
        commandInfo["tree"]   = {0, 0};
        commandInfo["search"] = {1, 1};
        commandInfo["cd"]     = {1, 1};
        commandInfo["mkdir"]  = {1, 1};
        commandInfo["touch"]  = {1, 1};
        commandInfo["rm"]     = {1, 1};
        commandInfo["rename"] = {2, 2};
        commandInfo["cp"]     = {2, 2};
        commandInfo["reindex"] = {0, 0};
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
        cout << "tree                   -> Show directory tree\n";
        cout << "search <name>          -> Search for files/folders\n";
        cout << "cd <dir>               -> Change directory\n";
        cout << "mkdir <name>           -> Create folder\n";
        cout << "touch <file>           -> Create file\n";
        cout << "rm <path>              -> Delete file/folder\n";
        cout << "rename <old> <new>     -> Rename item\n";
        cout << "cp <src> <dest>        -> Copy file\n";
        cout << "reindex                -> Rebuild file index\n";
        cout << "help                   -> Show help\n";
        cout << "exit                   -> Quit\n";

        cout << "----------------------------------\n\n";
    }

    // ==================== LIST FILES ====================

    string listFiles() {
        string output;

        try {

            for (const auto& entry : fs::directory_iterator(currentPath)) {

                if (fs::is_directory(entry.path())) {
                    output += "[DIR]  " +
                              entry.path().filename().string() +
                              "\n";
                }
                else {
                    output += "[FILE] " +
                              entry.path().filename().string() +
                              "\n";
                }
            }
        }

        catch (exception& e) {

            cout << RED << "Error: "
                 << e.what()
                 << RESET << endl;
        }

        return output;
    }

    // ==================== TREE ====================

    void showTree() {
        try {

            cout << CYAN << currentPath.filename().string()
                 << RESET << endl;

            showTreeRecursive(currentPath, "");
        }

        catch (exception& e) {

            cout << RED << "Error: "
                 << e.what()
                 << RESET << endl;
        }
    }

    void showTreeRecursive(const fs::path& path, const string& prefix) {

        for (const auto& entry : fs::directory_iterator(path)) {

            cout << prefix << "├── ";

            if (fs::is_directory(entry.path())) {

                cout << BLUE
                     << "[DIR] "
                     << entry.path().filename().string()
                     << RESET << endl;

                showTreeRecursive(entry.path(), prefix + "│   ");
            }
            else {

                cout << GREEN
                     << "[FILE] "
                     << entry.path().filename().string()
                     << RESET << endl;
            }
        }
    }

    string showTreeForSearch() {

        string output;

        try {

            for (const auto& entry :
                fs::recursive_directory_iterator(currentPath)) {

                if (entry.path().filename() == ".git")
                    continue;

                fs::path relativePath =
                    fs::relative(entry.path(), currentPath);

                if (fs::is_directory(entry.path())) {
                    output += "[DIR]  " +
                            relativePath.string() +
                            "\n";
                }
                else {
                    output += "[FILE] " +
                              relativePath.string() +
                              "\n";
                }
            }
        }

        catch (exception& e) {

            output += "Error: ";
            output += e.what();
            output += "\n";
        }

        return output;
    }

    // ==================== SEARCH FILE ====================

    string searchFiles(const string& keyword, const string& input = "") {

        string output;

        try {

            if (input.empty()) {

                lock_guard<mutex> lock(indexMutex);

                for (const auto& file : fileIndex) {

                    if (file.name.find(keyword) != string::npos) {
                        output += file.path.string() + "\n";
                    }
                }
            }

            else {

                stringstream ss(input);
                string line;

                while (getline(ss, line)) {

                    if (line.find(keyword) != string::npos) {
                        output += line + "\n";
                    }
                }
            }
        }

        catch (exception& e) {

            cout << RED
                 << "Error: "
                 << e.what()
                 << RESET << endl;
        }

        return output;
    }

    // ==================== CHANGE DIRECTORY ====================

    void changeDirectory(const string& dir) {

        fs::path newPath = currentPath / dir;

        if (fs::exists(newPath) && fs::is_directory(newPath)) {

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

            fs::path folderPath = currentPath / folderName;

            if (fs::exists(folderPath)) {
                cout << YELLOW
                     << "Folder already exists."
                     << RESET << "\n";
                return;
            }

            if (fs::create_directory(folderPath)) {

                {
                    lock_guard<mutex> lock(indexMutex);

                    FileEntry entry;
                    entry.name = folderName;
                    entry.path = folderPath;
                    entry.isDirectory = true;

                    fileIndex.push_back(entry);
                }

                cout << GREEN
                     << "Folder created."
                     << RESET << "\n";
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

    // ==================== CREATE FILE ====================

    void createFile(const string& fileName) {

        try {

            fs::path filePath = currentPath / fileName;

            if (fs::exists(filePath)) {
                cout << YELLOW
                     << "File already exists."
                     << RESET << "\n";
                return;
            }

            ofstream file(filePath);

            if (file) {

                file.close();

                {
                    lock_guard<mutex> lock(indexMutex);

                    FileEntry entry;
                    entry.name = fileName;
                    entry.path = filePath;
                    entry.isDirectory = false;

                    fileIndex.push_back(entry);
                }

                cout << GREEN
                     << "File created."
                     << RESET << "\n";
            }

            else {

                cout << RED
                     << "Failed to create file."
                     << RESET << "\n";
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

    // ==================== REMOVE FILE/FOLDER ====================

    void removeItem(const string& name) {

        try {

            fs::path target = currentPath / name;

            if (!fs::exists(target)) {
                cout << YELLOW
                     << "File or folder not found."
                     << RESET << "\n";
                return;
            }

            fs::remove_all(target);

            {
                lock_guard<mutex> lock(indexMutex);

                fileIndex.erase(
                    remove_if(
                        fileIndex.begin(),
                        fileIndex.end(),
                        [&](const FileEntry& entry) {

                            return entry.path == target ||
                                   entry.path.string().find(
                                       target.string() + "\\"
                                   ) == 0;
                        }
                    ),
                    fileIndex.end()
                );
            }

            cout << GREEN
                 << "Removed successfully."
                 << RESET << "\n";
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

    void renameItem(const string& oldName,const string& newName) {

        try {

            fs::path oldPath = currentPath / oldName;
            fs::path newPath = currentPath / newName;

            fs::rename(oldPath, newPath);

            {
                lock_guard<mutex> lock(indexMutex);

                for (auto& entry : fileIndex) {

                    if (entry.path == oldPath) {

                        entry.name = newName;
                        entry.path = newPath;
                    }

                    else if (entry.path.string().find(oldPath.string() + "\\") == 0) {

                        string relativePart = entry.path.string().substr(oldPath.string().length());

                        entry.path = newPath.string() + relativePart;
                    }
                }
            }

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

    void copyFile(const string& src,const string& dest) {

        try {

            fs::path sourcePath = currentPath / src;
            fs::path destinationPath = currentPath / dest;

            fs::copy_file(
                sourcePath,
                destinationPath,
                fs::copy_options::overwrite_existing
            );

            {
                lock_guard<mutex> lock(indexMutex);

                fileIndex.erase(
                    remove_if(
                        fileIndex.begin(),
                        fileIndex.end(),
                        [&](const FileEntry& entry) {
                            return entry.path == destinationPath;
                        }
                    ),
                    fileIndex.end()
                );

                FileEntry entry;
                entry.name = destinationPath.filename().string();
                entry.path = destinationPath;
                entry.isDirectory = false;

                fileIndex.push_back(entry);
            }

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

    bool validateCommand(const Command& command) {

        auto it = commandInfo.find(command.name);

        if (it == commandInfo.end())
            return false;

        int argumentCount = command.arguments.size();

        if (argumentCount < it->second.minArguments || argumentCount > it->second.maxArguments) {

            cout << YELLOW
                 << "Invalid number of arguments for '"
                 << command.name
                 << "'."
                 << RESET << endl;

            return false;
        }

        return true;
    }

    void buildIndex() {
        
        lock_guard<mutex> lock(indexMutex);

        fileIndex.clear();

        try {

            for (auto it = fs::recursive_directory_iterator(currentPath);
                it != fs::recursive_directory_iterator();
                ++it) {

                if (it->path().filename() == ".git") {
                    if (it->is_directory())
                        it.disable_recursion_pending();
                    continue;
                }

                FileEntry file;
                file.name = it->path().filename().string();
                file.path = it->path();
                file.isDirectory = it->is_directory();

                fileIndex.push_back(file);
            }
        }

        catch (const fs::filesystem_error& e) {

            cout << RED
                 << "Indexing error: "
                 << e.what()
                 << RESET << endl;
        }
    }
};

int main() {

    SetConsoleOutputCP(CP_UTF8);

    CLIFileManager manager;
    manager.start();
    return 0;
}