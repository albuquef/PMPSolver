#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <iostream>
#include <set>
#include <optional>
#include "toml.hpp"

class ConfigParser {
private:
    toml::table configFile;
    std::set<std::string>& configOverride;
    std::string configPath;

    /**
     * Returns the config file
     *
     * @param path - path to the config file
     */
    toml::table getConfigFile(const std::string& path) {
        toml::table file;

        try {
            file = toml::parse_file(path);
        } catch (const toml::parse_error& error) {
            std::cerr << "Error parsing config file: " << error << std::endl;
            throw; // Better to throw an exception rather than exiting
        }

        return file;
    }

    /**
     * Checks if the option isn't overridden
     *
     * @param option - the option to check
     */
    bool notOverriden(const std::string& option) const {
        return configOverride.find(option) == configOverride.end();
    }

public:
    /**
     * Instantiates ConfigParser
     *
     * @param configPath - path to the config file
     * @param configOverride - reference to a set of overridden options
     */
    ConfigParser(const std::string& configPath, std::set<std::string>& configOverride)
        : configOverride(configOverride), configPath(configPath) {
        configFile = getConfigFile(configPath);
    }

    /**
     * Returns the value at key
     *
     * @param key - the key to fetch the value for
     */
    template<typename T>
    std::optional<T> get(const std::string& key) const {
        std::optional<T> value;

        if (configFile.contains(key)) {
            const toml::node* node = configFile.get(key); // Use const toml::node*
            if (node) {
                value = node->value<T>();
            } else {
                std::cerr << "Error fetching value from config for \"" << key << "\", maybe invalid type conversion." << std::endl;
            }
        } else {
            std::cerr << "Error fetching value from config: Key \"" << key << "\" does not exist." << std::endl;
            throw std::runtime_error("Key does not exist"); // Better to throw an exception
        }

        return value;
    }

    /**
     * Sets option value according to config file
     *
     * @param option - pointer to the option to set
     * @param key - the option key
     */
    template<typename T>
    void setFromConfig(T* option, const std::string& key) {
        if (notOverriden(key)) {
            std::optional<T> opt = get<T>(key);

            if (opt) {
                *option = opt.value();
            }
        }
    }
};

#endif // CONFIG_PARSER_HPP
