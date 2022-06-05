#include <string>
#include <iostream>
#include "config_parser.hpp"
#include "toml.hpp"

ConfigParser::ConfigParser(std::string configPath) {
    try {
        configFile = toml::parse_file(configPath);
    } catch (const toml::parse_error& error) {
        std::cerr << "Error parsing config file: " << error << std::endl;
        exit(1);
    }
}
