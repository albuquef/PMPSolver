#include <string>
#include <iostream>
#include "config_parser.hpp"
#include "toml.hpp"

ConfigParser::ConfigParser(std::string configPath) {
    configFile = toml::parse_file(configPath);
}
