#include "io.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <algorithm>

/*
 * Parse the command line arguments into a map of argument names to values.
 * Arguments should be in the form "--name=value".
 * 
 * argc: The number of command line arguments.
 * argv: The array of command line arguments.
 * accepted_args: A list of valid argument names.
 * usage: A usage string to be displayed if the user provides incorrect arguments.
 */
std::map<std::string, std::string> parse_args(int argc, char** argv,
    const std::vector<std::string>& accepted_args, const std::string& usage) {
    std::map<std::string, std::string> args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        size_t equals_pos = arg.find('=');
        if (equals_pos == std::string::npos || arg.find("--") != 0) {
            std::cerr << "Error: Invalid argument: " << arg << std::endl;
            std::cerr << usage << std::endl;
            exit(1);
        }
        std::string name = arg.substr(2, equals_pos - 2);
        if (std::find(accepted_args.begin(), accepted_args.end(), name)
            == accepted_args.end()) {
            std::cerr << "Error: Invalid argument: " << name << std::endl;
            std::cerr << usage << std::endl;
            exit(1);
        }
        if (args.find(name) != args.end()) {
            std::cerr << "Error: Duplicate argument: " << name << std::endl;
            std::cerr << usage << std::endl;
            exit(1);
        }
        args[name] = arg.substr(equals_pos + 1);
    }
    return args;
}

/*
 * Given the path to a file on the filesystem, read the contents of the file
 * into a vector of bytes.
 */
std::vector<uint8_t> read_file_bytes(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + file_path);
    }
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

/*
 * Given a path to a file on the filesystem and a string, write the string to
 * the file.
 */
void write_file(const std::string& filename, const std::string& contents) {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filename);
    }
    file << contents;
}

/*
 * Given a vector of bytes, convert the bytes to a string of hex characters.
 */
std::string to_hex_string(const std::vector<uint8_t>& bytes) {
    std::ostringstream stream;
    for (uint8_t byte : bytes) {
        stream << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte);
    }
    return stream.str();
}
