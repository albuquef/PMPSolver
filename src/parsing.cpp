#include "parsing.hpp"

/*
 * Extract vector of uint_t values from file given by filename.
 * One value per line expected.
 */
vector<uint_t> get_vector(const string& filename) {
    vector<uint_t> vec;
    fstream file(filename);
    if (file.is_open()) {
        string str;
        while (getline(file, str)) {
            vec.push_back(stoi(str));
        }
    } else {
        cerr << "Error while opening file: " << filename << endl;
        exit(-1);
    }
    return vec;
}
