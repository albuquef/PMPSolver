#include "instance.hpp"

Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max)
        : locations(std::move(locations)), customers(std::move(customers)), dist_matrix(std::move(dist_matrix)), p(p),
          loc_max_id(loc_max), cust_max_id(cust_max) {
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
        loc_max_id = 0;
        cust_max_id = 0;
        // Process unique p_locations and customers
        cout << "Scanning input data...\n";
        auto start = tick();
        while (getline(loc_file, loc_str)) loc_max_id = max(loc_max_id, (uint_t) stoi(loc_str));
        while (getline(cust_file, cust_str)) cust_max_id = max(cust_max_id, (uint_t) stoi(cust_str));
        // Clear eof and fail flags
        loc_file.clear();
        cust_file.clear();
        // Go to beginning
        loc_file.seekg(0);
        cust_file.seekg(0);
        uint_t size = (loc_max_id + 1) * (cust_max_id + 1);
        cout << "Distance matrix dimensions: " << loc_max_id + 1 << " x " << cust_max_id + 1 << " = "
             << size << "\n";
        tock(start);
        // Preallocate distance matrix and loc, cust flag vectors
        dist_matrix = shared_ptr<dist_t[]>(new dist_t[size], std::default_delete<dist_t[]>());
        for (uint_t i = 0; i < size; i++) {
            dist_matrix[i] = numeric_limits<dist_t>::max();
        }
        vector<bool> loc_flags(loc_max_id + 1, false);
        vector<bool> cust_flags(cust_max_id + 1, false);
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
        // Extract unique p_locations and customers
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

uint_t Instance::getDistIndex(uint_t loc, uint_t cust) {
//    return loc * cust_max_id + cust;    // faster extraction of cust values
    return cust * loc_max_id + loc;    // faster extraction of loc values
}

void Instance::setDist(uint_t loc, uint_t cust, dist_t value) {
//    cout << loc << " " << cust << " " << value << endl;
    uint_t index = getDistIndex(loc, cust);
    dist_matrix[index] = value;
}

dist_t Instance::getDist(uint_t loc, uint_t cust) {
    uint_t index = getDistIndex(loc, cust);
    return dist_matrix[index];
}

Instance Instance::sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, default_random_engine *generator) {
    vector<uint_t> locations_new;
    vector<uint_t> customers_new;

    if (loc_cnt < locations.size()) {
        locations_new = getRandomSubvector(locations, loc_cnt, generator);
    } else {
        locations_new = locations;
    }
    if (cust_cnt < customers.size()) {
        customers_new = getRandomSubvector(customers, cust_cnt, generator);
    } else {
        customers_new = customers;
    }

    return Instance(locations_new, customers_new, dist_matrix, p_new, loc_max_id, cust_max_id);
}

void Instance::print() {
    cout << "Locations: ";
    for (auto l:locations) cout << l << " ";
    cout << endl;

    cout << "Customers: ";
    for (auto c:customers) cout << c << " ";
    cout << endl;

//    cout << "loc_max_id: " << loc_max_id << endl;
    cout << "loc_cnt: " << locations.size() << endl;
//    cout << "cust_max_id: " << cust_max_id << endl;
    cout << "cust_cnt: " << customers.size() << endl;
    cout << "p: " << p << endl << endl;
}

const vector<uint_t> &Instance::getCustomers() const {
    return this->customers;
}

const vector<uint_t> &Instance::getLocations() const {
    return this->locations;
}

uint_t Instance::get_p() const {
    return p;
}







