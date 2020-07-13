#include <any>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include "commons.hpp"

class ArgNotFoundError : std::runtime_error {
public:
    ArgNotFoundError(const std::string& arg_name)
	: std::runtime_error("Argument not found: " + arg_name)
    {
    }
};

class ArgsParser {
private:
    int m_argc;
    char** m_argv;
    struct arg_info_s {
	std::string short_name;
	std::string long_name;
	std::string description;
	int followup_vals;
	bool is_mandatory;
    };
    struct array_arg_info_s {
	std::string name;
	bool is_mandatory;
    };

    std::multimap<std::string, arg_info_s> m_arg_map;
    std::map<std::string, bool> m_mandatory_map;
    std::map<std::string, std::vector<std::string>> m_parsed_args;
    std::optional<array_arg_info_s> m_array_name;

    void insert(std::type_info& info, const std::string& raw_value)
    {
    }

public:
    ArgsParser(int argc, char** argv)
	: m_argc(argc)
	, m_argv(argv)
    {
    }
    ArgsParser& add_arg(const std::string& short_name, const std::string& long_name,
	const std::string& description, int expected_vals = 0, bool is_mandatory = false)
    {
	arg_info_s info { short_name, long_name, description, expected_vals, is_mandatory };
	m_arg_map.insert({ short_name, info });
	m_arg_map.insert({ long_name, info });
	if (is_mandatory)
	    m_mandatory_map[short_name] = false;
	return *this;
    }

    ArgsParser& add_array_arg(const std::string& mnemonic_name, bool is_mandatory)
    {
	m_array_name = array_arg_info_s { mnemonic_name, is_mandatory };
	return *this;
    }

    void parse()
    {
	m_parsed_args["program_name"].push_back(m_argv[0]);
	int i;
	for (int i; i < m_argc; i++) {
	    std::string cur_arg(m_argv[i]);
	    auto info_it = m_arg_map.find(cur_arg);
	    if (info_it == m_arg_map.end()) {
		break;
	    }
	    auto info = info_it->second;

	    m_parsed_args[info.short_name].clear();
	    if (info.is_mandatory)
		m_mandatory_map[info.short_name] = true;
	    for (int j = 0; j < info.followup_vals; j++) {
		i++;
		if (i == m_argc) {
		    TODO();
		}
		m_parsed_args[cur_arg].push_back(m_argv[i]);
	    }
	}

	if (m_array_name.has_value()) {
	    auto array_args_info = m_array_name.value();
	    if (array_args_info.is_mandatory)
		m_mandatory_map[array_args_info.name] = true;
	    for (; i < m_argc; i++) {
		m_parsed_args[array_args_info.name].push_back(m_argv[i]);
	    }
	}
    }

    bool is_set(const std::string& name)
    {
	return m_parsed_args.find(name) != m_parsed_args.end();
    }

    template <typename T>
    T& get(const std::string& name)
    {
	auto el = m_parsed_args.find(name);
	if (el == m_parsed_args.end())
	    TODO();

	auto val = el->second;
	auto stream = std::stringstream(val.front());
	T ret_val;
	stream >> ret_val;
	return ret_val;
    }

    template <typename T>
    std::vector<T> get_vector(const std::string& name)
    {
	auto el = m_parsed_args.find(name);
	if (el == m_parsed_args.end())
	    TODO();

	auto val = el->second;
	std::vector<T> return_value;
	for (const auto& arg : val) {
	    auto stream = std::stringstream(arg);
	    T ret_val;
	    stream >> ret_val;
	    return_value.push_back(ret_val);
	}
	return return_value;
    };
