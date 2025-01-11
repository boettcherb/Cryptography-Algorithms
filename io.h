#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <map>

std::map<std::string, std::string> parse_args(int argc, char** argv, const std::string& usage);
std::vector<uint8_t> read_file_bytes(const std::string& filename);
void write_file(const std::string& filename, const std::string& contents);
std::string to_hex_string(const std::vector<uint8_t>& bytes);
