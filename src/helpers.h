//
// Created by crax on 7/31/20.
//

#pragma once

#include "exceptions.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace CL {
class Helpers {
public:
	static std::string read_file_content(const std::string &path) {
		auto file_stream = std::ifstream(path);
		if(!file_stream.is_open()) {
			throw FileNotFoundException(path);
		}

		std::string content, line;
		while (std::getline(file_stream, line)) {
			content += line + "\n";
		}
		return content;
	}

	static std::vector<std::string> split_into_lines(const std::string &string) {
		std::vector<std::string> lines;
		std::string line;
		std::stringstream stream(string);
		while (std::getline(stream, line)) {
			lines.push_back(line);
		}
		return lines;
	}

	static std::vector<std::string> read_into_lines(const std::string &path) {
		auto file_stream = std::ifstream(path);
		if(!file_stream.is_open()) {
			throw FileNotFoundException(path);
		}

		std::string line;
		std::vector<std::string> lines;
		while (std::getline(file_stream, line)) {
			lines.push_back(line);
		}
		return lines;
	}
};

template<class, class, typename = void>
struct is_explicitly_convertible
	: std::false_type {
};

template<class From, class To>
struct is_explicitly_convertible<From,
								 To,
								 std::void_t<decltype(static_cast<To>(std::declval<
									 From>()))>>
	: std::true_type {
};

template<class T>
std::string addr_to_hex_str(const T &el) {
	std::stringstream str;
	str << "@0x" << std::hex;
	str << reinterpret_cast<uint64_t>(&el);
	return str.str();
}
}
