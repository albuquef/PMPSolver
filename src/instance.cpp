#include "instance.hpp"

Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max)
        : locations(std::move(locations)), customers(std::move(customers)), dist_matrix(std::move(dist_matrix)), p(p),
          loc_max(loc_max), cust_max(cust_max) {
}

Instance::Instance(const string &loc_filename, const string &cust_filename, const string &dist_filename, uint_t p) : p(
        p) {
    fstream loc_file(loc_filename);
    fstream cust_file(cust_filename);
    fstream dist_file(dist_filename);

    if (loc_file.is_open() && cust_file.is_open() && dist_file.is_open()) {
        string loc_str;
        string cust_str;
        string dist_str;
        loc_max = 0;
        cust_max = 0;
        // Process unique locations and customers
        cout << "Scanning input data...\n";
        auto start = tick();
        while (getline(loc_file, loc_str)) loc_max = max(loc_max, (uint_t) stoi(loc_str));
        while (getline(cust_file, cust_str)) cust_max = max(cust_max, (uint_t) stoi(cust_str));
        // Clear eof and fail flags
        loc_file.clear();
        cust_file.clear();
        // Go to beginning
        loc_file.seekg(0);
        cust_file.seekg(0);
        cout << "Distance matrix dimensions: " << loc_max + 1 << " x " << cust_max + 1 << " = "
             << (loc_max + 1) * (cust_max + 1) << "\n";
        tock(start);
        // Preallocate distance matrix and loc, cust flag vectors
        dist_matrix = shared_ptr<dist_t[]>(new dist_t[(loc_max + 1) * (cust_max + 1)], std::default_delete<dist_t[]>());
        vector<bool> loc_flags(loc_max + 1, false);
        vector<bool> cust_flags(cust_max + 1, false);
        // Fill it
        cout << "Loading distance matrix...\n";
        start = tick();
        while (getline(dist_file, dist_str)) {
            getline(loc_file, loc_str);
            getline(cust_file, cust_str);
            auto loc = stoi(loc_str);
            auto cust = stoi(cust_str);
            setDist(loc, cust, stod(dist_str));
            loc_flags[loc] = true;
            cust_flags[cust] = true;
        }
        // Extract unique locations and customers
        for (uint_t loc = 0; loc < loc_flags.size(); loc++) {
            if (loc_flags[loc]) locations.push_back(loc);
        }
        for (uint_t cust = 0; cust < cust_flags.size(); cust++) {
            if (cust_flags[cust]) customers.push_back(cust);
        }
        cout << "locations: " << locations.size() << endl;
        cout << "customers: " << customers.size() << endl;
        cout << "p: " << p << endl;
        tock(start);
    } else {
        cerr << "Error while opening some of the input files\n";
        exit(-1);
    }
}

void Instance::setDist(uint_t loc, uint_t cust, dist_t value) {
    uint_t index = loc * cust_max + cust;
    dist_matrix[index] = value;
}

dist_t Instance::getDist(uint_t loc, uint_t cust) {
    uint_t index = loc * cust_max + cust;
    return dist_matrix[index];
}

Instance Instance::sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, default_random_engine *generator) {
    auto locations_new = sampleSubvector(&locations, loc_max, loc_cnt, generator);
    auto customers_new = sampleSubvector(&customers, cust_max, cust_cnt, generator);
    return Instance(locations_new, customers_new, dist_matrix, p_new, loc_max, cust_max);
}

void Instance::print() {
    cout << "Locations: ";
    for (auto l:locations) cout << l << " ";
    cout << endl;

    cout << "Customers: ";
    for (auto c:customers) cout << c << " ";
    cout << endl;

    cout << "loc_max: " << loc_max << endl;
    cout << "loc_cnt: " << locations.size() << endl;
    cout << "cust_max: " << cust_max << endl;
    cout << "cust_cnt: " << customers.size() << endl;
    cout << "p: " << p << endl << endl;
}



