#include <string>
#include <iostream>
#include <set>
#include <optional>

#include "globals.hpp"
#include "toml.hpp"


class ConfigParser {

    private:
        toml::table configFile;
        std::set<const char*> configOverride;

    private:
        /**
         * Returns the config file
         *
         * @param path
         */
        toml::table getConfigFile(std::string path) {
            toml::table file;

            try {
                file = toml::parse_file(path);
            } catch (const toml::parse_error& error) {
                std::cerr << "Error parsing config file: " << error << std::endl;
                exit(1);
            }

            return file;
        }

        /**
         * Tells if the option isn't overriden
         *
         * @param option
         */
        bool notOverriden(const char* option) {
            return !configOverride.contains(option);
        }

    public:
        /**
         * Instanciates ConfigParser
         *
         * @param configPath - path to the config file
         */
        ConfigParser(std::string configPath, 
                     std::set<const char*> configOverride) {
            configFile = getConfigFile(configPath);
            this->configOverride = configOverride;
        }

        /**
         * Returns the value at key
         *
         * @param key
         */
        template<typename T>
        std::optional<T> get(std::string key) {
            std::optional<T> value;

            if (configFile.contains(key)) {
                toml::node* node = configFile.get(key);
                value = node->value<T>();
            } else {
                std::cerr << "Error fetching value from config: Key \"" << key << "\" does not exist." << std::endl;
                exit(1);
            }

            if (!value) {
                std::cerr << "Error fetching value from config for \"" << key << "\", maybe invalid type conversion." << std::endl;
            }

            return value;
        }

        /**
         * Sets option value according to config file
         *
         * @param option - the option to set
         * @param key - the option key
         */
        template<typename T>
        void setFromConfig(T* option, const char* key) {
            if (notOverriden(key)) {
                std::optional<T> opt = get<T>(key);

                if (opt) {
                    *option = opt.value();
                }
            }
        }

};