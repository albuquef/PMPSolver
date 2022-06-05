#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include "toml.hpp"

class ConfigParser {

    private:
        std::string configPath;
        toml::parse_result configFile;

    public:
        /**
         * Instanciates ConfigParser
         *
         * @param configPath - path to the config file
         */
        ConfigParser(std::string);

};

#endif  // CONFIG_PARSER_HPP