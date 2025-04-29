#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <filesystem>
#include <regex>
#include <cctype>
#include <cstdlib>
#include <limits.h>
#include <windows.h>
#include <algorithm>

// helper methods here:

using namespace std;

string cleanLine(string line) {
	size_t pos = line.find("//");
	if (pos != string::npos) {
		line = line.substr(0, pos);
	};
	line.erase(0, line.find_first_not_of(" \t\n\r"));
	line.erase(line.find_last_not_of(" \t\n\r") + 1);
	return line;
};

string readFile(const string& filePath) {
	ifstream file(filePath);
	if (!file) {
		cerr << "Error: Unable to open file " << filePath << endl;
		exit(1);
	};
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
};

vector<string> getLines(const string& codeStr) {
	vector<string> lines;
	stringstream ss(codeStr);
	string line;
	while (getline(ss, line)) {
		if (!line.empty()) {
			lines.push_back(line);
		};
	};
	return lines;
};

std::string getFileName(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of(".");
    if (slash == std::string::npos) slash = -1;
    if (dot == std::string::npos || dot < slash) dot = path.size();
    return path.substr(slash + 1, dot - slash - 1);
};

std::string compileCommand(const std::string& filePath, const std::string& extension) {
	std::string fileName = getFileName(filePath);
	std::string command = "nasm -c " + filePath;

	std::string libPath = getLibPath();
	command += " -I \"" + libPath + "\"";

	for (const auto& entry : std::filesystem::directory_iterator(libPath)) {
		if (entry.path().extension() == ".o") {
			command += " " + entry.path().string();
		};
	};

	command += " -o " + fileName + "." + extension;

	return command;
};

std::string getLibPath() {
	char buffer[MAX_PATH];
	if (GetModuleFileNameA(NULL, buffer, MAX_PATH) == 0) {
		std::cerr << "Error: Unable to determine executable path" << std::endl;
		exit(1);
	};

	std::filesystem::path exePath(buffer);
	std::filesystem::path libPath = exePath.parent_path() / ".." / "lib";
	return libPath.string();
};

struct FuncCall {
    std::string funcName;
    std::vector<std::string> args;
};

FuncCall parseFuncCall(const std::string& input) {
    FuncCall result;

    size_t openParen = input.find('(');
    if (openParen == std::string::npos) return result;

    // Extract the function name
    result.funcName = input.substr(0, openParen);
    result.funcName.erase(0, result.funcName.find_first_not_of(" \t\n"));
    result.funcName.erase(result.funcName.find_last_not_of(" \t\n") + 1);

    // Extract the argument string
    std::string argStr = input.substr(openParen + 1, input.rfind(')') - openParen - 1);
    std::string currentArg;
    int depthParen = 0, depthBrace = 0, depthBracket = 0;
    bool inString = false;
    bool inEscape = false;

    // Loop through the argument string
    for (size_t i = 0; i < argStr.size(); ++i) {
        std::string c = argStr.substr(i, 1);  // Extract current character as a string

        // Handle the start and end of string literals
        if (c == "\"" && !inEscape) {
            inString = !inString;
        };

        // Handle escape character inside strings
        if (inString && c == "\\" && !inEscape) {
            inEscape = true;
            currentArg += c;
            continue;
        };

        if (inString && inEscape) {
            inEscape = false;
        };

        // Handle structure depths (for parentheses, braces, brackets)
        if (!inString) {
            if (c == "(") depthParen++;
            if (c == ")") depthParen--;
            if (c == "{") depthBrace++;
            if (c == "}") depthBrace--;
            if (c == "[") depthBracket++;
            if (c == "]") depthBracket--;

            // Add argument on encountering a comma at top-level depth
            if (c == "," && depthParen == 0 && depthBrace == 0 && depthBracket == 0) {
                result.args.push_back(currentArg);
                currentArg.clear();
                continue;
            };
        };

        currentArg += c;  // Add character to current argument
    };

    if (!currentArg.empty()) {
        result.args.push_back(currentArg);  // Add last argument if it exists
    };

    return result;
};

// compiler

string compile(vector<string> codeLines) {
        vector<string> vars;
        string asmOut;

        asmOut += "section .text\n global start\n extern ExitProcess\n\nstart:\n call main\n call ExitProcess\n";

        for (string line : codeLines) {
                line = cleanLine(line);
                if (line.substr(8) == "#include") {
                        asmOut; 
                };
        };

	return 0;
};

// main method

int main(int argc, char* argv[]) {
	string filePath = argv[1];
	string fileContent = readFile(filePath);
	vector lines = getLines(fileContent);
	string code = compile(lines);
	string extension = argv[2];
	string command = compileCommand(filePath, extension);
	system(command.c_str());

	return 0;
};
