#include "instance.hpp"

#include <utility>
#include <sstream>

Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights,
                   shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max)
        : locations(std::move(locations)), customers(std::move(customers)), cust_weights(std::move(cust_weights)),
          dist_matrix(std::move(dist_matrix)), p(p),
          loc_max_id(loc_max), cust_max_id(cust_max) {
}

Instance::Instance(const string &loc_filename, const string &cust_filename, const string &dist_filename,
                   const string &weights_filename, uint_t p) : p(
        p) {
    fstream loc_file(loc_filename);
    fstream cust_file(cust_filename);
    fstream dist_file(dist_filename);
    fstream weights_file(weights_filename);

    if (loc_file.is_open() && cust_file.is_open() && dist_file.is_open() && weights_file.is_open()) {
        string loc_str;
        string cust_str;
        string dist_str;
        string weight_str;
        loc_max_id = 0;
        cust_max_id = 0;
        // Process unique locations and customers
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
            dist_matrix[i] = DEFAULT_DISTANCE;
        }
        vector<bool> loc_flags(loc_max_id + 1, false);
        vector<bool> cust_flags(cust_max_id + 1, false);
        // Fill it
        cout << "Loading distance matrix...\n";
        dist_t sum = 0; // sum of distances
        dist_t sum_sq = 0; // sum of squared distances
        uint_t cnt = 0;
        start = tick();
        while (getline(dist_file, dist_str)) {
            getline(loc_file, loc_str);
            getline(cust_file, cust_str);
            auto loc = stoi(loc_str);
            auto cust = stoi(cust_str);
            dist_t dist = stod(dist_str);
            setDist(loc, cust, dist);
            loc_flags[loc] = true;
            cust_flags[cust] = true;
            sum += dist;
            sum_sq += dist * dist;
            cnt++;
        }
        // Determine stdev and bandwidth
        dist_t mean = sum / cnt;
        dist_t variance = sum_sq / cnt - mean * mean;
        dist_t stdev = sqrt(variance);
        dist_t a = (4 * pow(stdev, 5)) / (3 * cnt);
        dist_t b = 0.2;
        h = pow(a, b);
        cout << "dists stdev: " << stdev << endl;
        cout << "bandwidth h: " << h << endl;
        // Extract unique p_locations and customers
        cust_weights = shared_ptr<dist_t[]>(new dist_t[cust_max_id + 1], std::default_delete<dist_t[]>());
        for (uint_t loc = 0; loc < loc_flags.size(); loc++) {
            if (loc_flags[loc]) locations.push_back(loc);
        }
        for (uint_t cust = 0; cust < cust_flags.size(); cust++) {
            cust_weights[cust] = DEFAULT_WEIGHT;
            if (cust_flags[cust]) {
                customers.push_back(cust);
                getline(weights_file, weight_str);
                dist_t weight = stod(weight_str);
                cust_weights[cust] = weight;
            }
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

vector<string> tokenize(string input, char delim) {
    vector <string> tokens;
    stringstream check1(input);
    string intermediate;
    while(getline(check1, intermediate, delim))
    {
        tokens.push_back(intermediate);
    }
    return tokens;
}

Instance::Instance(const string &dist_matrix_filename, const string &weights_filename, uint_t p, char delim) : p(p) {
    // Open streams
    fstream dist_matrix_file(dist_matrix_filename);
    fstream weights_file(weights_filename);
    //
    if (dist_matrix_file.is_open() && weights_file.is_open()) {
        loc_max_id = 0;
        cust_max_id = 0;
        string line;
        // Scan data to determine distance matrix dimensions
        cout << "Scanning input data...\n";
        auto start = tick();
        getline(dist_matrix_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        while (getline(dist_matrix_file, line)) {
            auto tokens = tokenize(line, delim);
            cust_max_id = max(cust_max_id, (uint_t) stoi(tokens[0]));
            loc_max_id = max(loc_max_id, (uint_t) stoi(tokens[1]));
        }
        // Clear eof and fail flags, go to beginning
        dist_matrix_file.clear();
        dist_matrix_file.seekg(0);
        uint_t size = (loc_max_id + 1) * (cust_max_id + 1);
        cout << "Distance matrix dimensions: " << loc_max_id + 1 << " x " << cust_max_id + 1 << " = " << size << "\n";
        tock(start);
        // Load weights
        start = tick();
        cout << "Loading weights...\n";
        cust_weights = shared_ptr<dist_t[]>(new dist_t[cust_max_id + 1], std::default_delete<dist_t[]>());
        for (uint_t cust = 0; cust < cust_max_id + 1; cust++) cust_weights[cust] = DEFAULT_WEIGHT;
        getline(weights_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t w_cnt = 0;
        while (getline(weights_file, line)) {
            auto tokens = tokenize(line, delim);
            auto cust = stoi(tokens[0]);
            auto weight = stod(tokens[1]);
            cust_weights[cust] = weight;
            w_cnt++;
        }
        cout << "Loaded " << w_cnt << " weights\n";
        tock(start);
        // Preallocate distance matrix and loc, cust flag vectors
        start = tick();
        dist_matrix = shared_ptr<dist_t[]>(new dist_t[size], std::default_delete<dist_t[]>());
        for (uint_t i = 0; i < size; i++) {
            dist_matrix[i] = DEFAULT_DISTANCE;
        }
        vector<bool> loc_flags(loc_max_id + 1, false);
        vector<bool> cust_flags(cust_max_id + 1, false);
        // Fill it
        cout << "Loading distance matrix...\n";
        dist_t sum = 0; // sum of distances
        dist_t sum_sq = 0; // sum of squared distances
        uint_t cnt = 0;
        getline(dist_matrix_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        while (getline(dist_matrix_file, line)) {
            auto tokens = tokenize(line, delim);
            auto cust = stoi(tokens[0]);
            auto loc = stoi(tokens[1]);
            dist_t dist = stod(tokens[2]);
            setDist(loc, cust, dist);
            loc_flags[loc] = true;
            cust_flags[cust] = true;
            sum += dist;
            sum_sq += dist * dist;
            cnt++;
        }
        // Determine stdev and bandwidth
        dist_t mean = sum / cnt;
        dist_t variance = sum_sq / cnt - mean * mean;
        dist_t stdev = sqrt(variance);
        dist_t a = (4 * pow(stdev, 5)) / (3 * cnt);
        dist_t b = 0.2;
        h = pow(a, b);
        cout << "Loaded " << cnt << " distances\n";
        cout << "dists stdev: " << stdev << endl;
        cout << "bandwidth h: " << h << endl;
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


uint_t Instance::getDistIndex(uint_t loc, uint_t cust) {
//    return loc * cust_max_id + cust;    // faster extraction of cust values
    return cust * loc_max_id + loc;    // faster extraction of loc values
}

void Instance::setDist(uint_t loc, uint_t cust, dist_t value) {
    uint_t index = getDistIndex(loc, cust);
    dist_matrix[index] = value;
}


dist_t Instance::getCustWeight(uint_t cust) {
    return cust_weights[cust];
}

dist_t Instance::getWeightedDist(uint_t loc, uint_t cust) {
    uint_t index = getDistIndex(loc, cust);
    return cust_weights[cust] * dist_matrix[index];
}

dist_t Instance::getRealDist(uint_t loc, uint_t cust) {
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

    return Instance(locations_new, customers_new, cust_weights, dist_matrix, p_new, loc_max_id, cust_max_id);
}

void Instance::print() {
//    cout << "Locations: ";
//    for (auto l:locations) cout << l << " ";
//    cout << endl;
//
//    cout << "Customers: ";
//    for (auto c:customers) cout << c << " ";
//    cout << endl;

    cout << "loc_max_id: " << loc_max_id << endl;
    cout << "loc_cnt: " << locations.size() << endl;
    cout << "cust_max_id: " << cust_max_id << endl;
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

uint_t Instance::getClosestCust(uint_t loc) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    uint_t cust_cl;
    for (auto cust:customers) {
        auto dist = getRealDist(loc, cust);
        if (dist <= dist_min) {
            dist_min = dist;
            cust_cl = cust;
        }
    }
    return cust_cl;
}

double Instance::getVotingScore(uint_t loc, uint_t cust) {
    auto dist = getRealDist(loc, cust);
    double score = 0;
    if (dist <= BW_CUTOFF * h) {
        score = exp(-(dist * dist) / (h * h));
    }
    return score;
}

Instance Instance::getReducedSubproblem(const vector<uint_t> &locations_new) {
    return Instance(locations_new, customers, cust_weights, dist_matrix, p, loc_max_id, cust_max_id);
}








