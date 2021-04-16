#include <fstream>
#include <iostream>
#include "instance.hpp"

Instance::Instance(const string& loc_filename, const string& cust_filename, const string& dist_filename, uint_t p): p(p) {
    fstream loc_file(loc_filename);
    fstream cust_file(cust_filename);
    fstream dist_file(dist_filename);

    if (loc_file.is_open() && cust_file.is_open() && dist_file.is_open()) {
        cout << "Reading input files...\n";
        string loc_str;
        string cust_str;
        string dist_str;
        // Process unique locations and customers
        while (getline(loc_file, loc_str)) locations.insert(stoi(loc_str));
        while (getline(cust_file, cust_str)) customers.insert(stoi(cust_str));
        // Clear eof and fail flags
        loc_file.clear();
        cust_file.clear();
        // Go to beginning
        loc_file.seekg(0);
        cust_file.seekg(0);
        cout << "locations: " << locations.size() << endl;
        cout << "customers: " << customers.size() << endl;
        cout << "p: " << p << endl;
        // Preallocate distance matrix
        loc_max = *locations.rbegin();
        cust_max = *customers.rbegin();
        dist_matrix = new dist_t [(loc_max + 1) * (cust_max + 1)];
        // Fill it
        while (getline(loc_file, loc_str)) {
            getline(cust_file, cust_str);
            getline(dist_file, dist_str);
            setDist(stoi(loc_str), stoi(cust_str), stod(dist_str));
        }
        cout << "Distance matrix loaded, dimensions: " << loc_max + 1 << " x " << cust_max + 1 << " = " << (loc_max + 1) * (cust_max + 1) << "\n";
    } else {
        cerr << "Error while opening some of the input files\n";
        exit(-1);
    }
}

Instance::~Instance() {
    delete[] dist_matrix;
}

void Instance::setDist(uint_t loc, uint_t cust, dist_t value) {
    uint_t index = loc * cust_max + cust;
    dist_matrix[index] = value;
}

dist_t Instance::getDist(uint_t loc, uint_t cust) {
    uint_t index = loc * cust_max + cust;
    return dist_matrix[index];
}

