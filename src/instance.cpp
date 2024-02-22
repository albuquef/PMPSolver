#include "instance.hpp"

#include <utility>
#include <sstream>
#include <string.h>

// Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights,
//                    shared_ptr<dist_t[]> dist_matrix, shared_ptr<dist_t[]> loc_capacities, uint_t p,
//                    uint_t loc_max, uint_t cust_max, string type_service)
//         : locations(locations), customers(customers), cust_weights(cust_weights),
//           dist_matrix(dist_matrix),
//           loc_capacities(loc_capacities), p(p),
//           loc_max_id(loc_max), cust_max_id(cust_max), type_service(type_service){
//     total_demand = 0;
//     for (auto cust:this->customers) {
//         total_demand += this->getCustWeight(cust);
//     }
// }

Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights,
                   shared_ptr<dist_t[]> loc_capacities, shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max, string type_service)
        : locations(locations), customers(customers), cust_weights(cust_weights),
          loc_capacities(loc_capacities),dist_matrix(dist_matrix),
          p(p),loc_max_id(loc_max), cust_max_id(cust_max), type_service(type_service){
    total_demand = 0;
    for (auto cust:this->customers) {
        total_demand += this->getCustWeight(cust);
    }
}

Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights,
                   shared_ptr<dist_t[]> loc_capacities, shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max, string type_service, unordered_set<uint_t> unique_subareas, shared_ptr<uint_t[]> loc_coverages, string type_subarea)
        : locations(locations), customers(customers), cust_weights(cust_weights),
          loc_capacities(loc_capacities),dist_matrix(dist_matrix),
          p(p),loc_max_id(loc_max), cust_max_id(cust_max), type_service(type_service), 
          unique_subareas(unique_subareas), loc_coverages(loc_coverages), type_subarea(type_subarea){
    total_demand = 0;
    for (auto cust:this->customers) {
        total_demand += this->getCustWeight(cust);
    }
}

vector<string> tokenize(const string& input, char delim) {
    vector <string> tokens;
    stringstream check1(input);
    string intermediate;
    while(getline(check1, intermediate, delim))
    {
        tokens.push_back(intermediate);
    }
    return tokens;
}

Instance::Instance(const string &dist_matrix_filename, const string &weights_filename, const string& capacities_filename, uint_t p, char delim, string type_service) : p(p), type_service(type_service) {

    // Open streams
    fstream dist_matrix_file(dist_matrix_filename);
    fstream weights_file(weights_filename);
    fstream capacities_file(capacities_filename);
    //
    if (dist_matrix_file.is_open() && weights_file.is_open() && capacities_file.is_open()) {
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
        total_demand = 0;
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
            total_demand += weight;
            w_cnt++;
        }
        cout << "Loaded " << w_cnt << " weights\n";
        cout << "Total customer demand: " << total_demand << endl;
        tock(start);
        // Load capacities
        start = tick();
        cout << "Loading capacities...\n";
        loc_capacities = shared_ptr<dist_t[]>(new dist_t[loc_max_id + 1], std::default_delete<dist_t[]>());
        for (uint_t loc = 0; loc < loc_max_id + 1; loc++) loc_capacities[loc] = DEFAULT_CAPACITY;
        getline(capacities_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t cap_cnt = 0;
        while (getline(capacities_file, line)) {
            auto tokens = tokenize(line, delim);
            auto loc = stoi(tokens[0]);
            auto cap = stod(tokens[1]);
            loc_capacities[loc] = cap;
            cap_cnt++;
        }
        cout << "Loaded " << cap_cnt << " capacities\n";
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
        cerr << "Error while trying to open the following input files :\n";

        if(!dist_matrix_file) cerr << "- Distance matrix" << endl;
        if(!weights_file) cerr << "- Weights file" << endl;
        if(!capacities_file) cerr << "- Capacities file" << endl;
        
        cerr << "Check if the path is correct and/or the correct name was given for the concerned files." << endl;
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


Instance Instance::sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, uint_t seed) {
    

    // Create a new engine and seed it
    std::default_random_engine generator(seed);

    vector<uint_t> locations_new;
    vector<uint_t> customers_new;

    if (loc_cnt < locations.size()) {
        locations_new = getRandomSubvector(locations, loc_cnt, &generator);
    } else {
        locations_new = locations;
    }
    if (cust_cnt < customers.size()) {
        customers_new = getRandomSubvector(customers, cust_cnt, &generator);
    } else {
        customers_new = customers;
    }


    if (!cover_mode) {
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service);
    }

    return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);
}

void Instance::print() {
//    cout << "Locations: ";
//    for (auto l:locations) cout << l << " ";
//    cout << endl;

//    cout << "Customers: ";
//    for (auto c:customers) cout << c << " ";
//    cout << endl;

    cout << "loc_max_id: " << loc_max_id << endl;
    cout << "loc_cnt: " << locations.size() << endl;
    cout << "service: " << type_service << endl;
    cout << "cust_max_id: " << cust_max_id << endl;
    cout << "cust_cnt: " << customers.size() << endl;
    if (cover_mode) {
        cout << "cover_max_id: " << cover_max_id << endl;
        cout << "subareas_cnt: " << unique_subareas.size() << endl;
        cout << "subareas: "<< type_subarea << endl;
    }
    cout << "p: " << p << endl;
    cout << "total_demand: " << total_demand << endl << endl;
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
    uint_t cust_cl = numeric_limits<uint_t>::max();
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

Instance Instance::getReducedSubproblem(const vector<uint_t> &locations_new, string type_service) {
    return Instance(locations_new, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id, type_service);
}

dist_t Instance::getLocCapacity(uint_t loc) {
    return loc_capacities[loc];
}

dist_t Instance::getTotalDemand() const {
    return total_demand;
}

string Instance::getTypeService() const {
    return type_service;
}


void Instance::ReadCoverages(const string& coverages_filename,string type_subarea,char delim){
    this->type_subarea = type_subarea;
    fstream coverages_file(coverages_filename);
    cover_max_id = 0;
    if (coverages_file.is_open()) {
        string line;
        // Load coverages
        auto start = tick();
        cout << "Loading coverages...\n";
        loc_coverages = shared_ptr<uint_t[]>(new uint_t[loc_max_id + 1], default_delete<uint_t[]>());
        for (uint_t loc = 0; loc < loc_max_id + 1; loc++) loc_coverages[loc] = 0;
        getline(coverages_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t cover_cnt = 0; 
        vector<uint_t> qtd_coverages; // to store the number of coverages per subarea
        while (getline(coverages_file, line)) {
            auto tokens = tokenize(line, delim);
            auto loc = stoi(tokens[0]);
            auto subarea = stoi(tokens[1]);
            cover_max_id = max(cover_max_id, (uint_t) subarea);
            loc_coverages[loc] = subarea;
            unique_subareas.insert(subarea);
            cover_cnt++;
        }
        cout << "Loaded " << cover_cnt << " locations covered\n";
        cout << "Number of subareas: " << unique_subareas.size() << endl;

        // Sttistics about the number of coverages per subarea
        for (auto subarea:unique_subareas) {
            // cout << "Subarea " << subarea << " has " << count(loc_coverages.get(), loc_coverages.get() + loc_max_id + 1, subarea) << " locations" << endl;
            qtd_coverages.push_back(count(loc_coverages.get(), loc_coverages.get() + loc_max_id + 1, subarea));
        }
        // Initialize max and min with the first element of the vector
        uint_t max = qtd_coverages[0];
        uint_t min = qtd_coverages[0];
        uint_t sum = 0.0;
        // Find max, min, and sum of elements
        for (uint_t num : qtd_coverages) {
            if (num > max) {
                max = num;
            }
            if (num < min) {
                min = num;
            }
            sum += num;
        }
        
        // Calculate average
        double average = sum / qtd_coverages.size();
        cout << "Average number of locations for subarea: " << average << endl;
        cout << "Max qte locations for one subarea: " << max << endl;
        cout << "Min qte locations for one subarea: " << min << endl;

        tock(start);
    }else{
        cerr << "Error while trying to open the subareas file" << endl;
        cout << "subareas filename: " << coverages_filename << endl;
    }
}

string Instance::getTypeSubarea() const {
    return type_subarea;
}

uint_t Instance::getSubareaLocation(uint_t loc){
    return loc_coverages[loc];
}

unordered_set<uint_t> Instance::getSubareasSet(){
    return unique_subareas;
}

const vector<uint_t> Instance::getLocationsSubarea(uint_t subarea){
    vector<uint_t> vet;
    for(uint_t loc = 0; loc < loc_max_id + 1; loc++){
        if(loc_coverages[loc] == subarea)
            vet.push_back(loc);
    }
    return vet;
}

bool Instance::isInTheSameSubarea(uint_t loc1, uint_t loc2){
    return loc_coverages[loc1] == loc_coverages[loc2];
}

void Instance::setCoverModel(bool cover_mode){
    this->cover_mode = cover_mode;
}

bool Instance::isCoverMode() {
    return cover_mode;
}
