#include "instance.hpp"

#include <utility>
#include <sstream>
#include <string.h>
#include <cmath> // Required for ceil()



Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights,
                   shared_ptr<dist_t[]> loc_capacities, shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max, string type_service)
        : locations(locations), customers(customers), cust_weights(cust_weights),
          loc_capacities(loc_capacities),dist_matrix(dist_matrix),
          p(p),loc_max_id(loc_max), cust_max_id(cust_max), type_service(type_service){
    total_demand = 0.0;
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
    cover_mode = true;
    total_demand = 0.0;
    for (auto cust:this->customers) {
        total_demand += this->getCustWeight(cust);
    }
    this->cover_max_id = unique_subareas.size();
}

Instance::Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights,
                   shared_ptr<dist_t[]> loc_capacities, shared_ptr<dist_t[]> dist_matrix, uint_t p,
                   uint_t loc_max, uint_t cust_max, string type_service, unordered_set<uint_t> unique_subareas, shared_ptr<uint_t[]> loc_coverages, string type_subarea, unordered_set<uint_t> unique_subareas_n2, shared_ptr<uint_t[]> loc_coverages_n2, string type_subarea_n2)
        : locations(locations), customers(customers), cust_weights(cust_weights),
          loc_capacities(loc_capacities),dist_matrix(dist_matrix),
          p(p),loc_max_id(loc_max), cust_max_id(cust_max), type_service(type_service), 
          unique_subareas(unique_subareas), loc_coverages(loc_coverages), type_subarea(type_subarea),
          unique_subareas_n2(unique_subareas_n2), loc_coverages_n2(loc_coverages_n2), type_subarea_n2(type_subarea_n2){
    cover_mode = true;
    cover_mode_n2 = true;
    total_demand = 0.0;
    for (auto cust:this->customers) {
        total_demand += this->getCustWeight(cust);
    }
    this->cover_max_id = unique_subareas.size();
    this->cover_n2_max_id = unique_subareas_n2.size();
    cout << "type subarea n2: " << type_subarea_n2 << endl;
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

// Instance constructor with distance matrix 
Instance::Instance(const string &dist_matrix_filename, const string &weights_filename, const string& capacities_filename, uint_t p, char delim, string type_service, uint_t cust_max_id, uint_t loc_max_id) : p(p), type_service(type_service) {

    if (strcmp(dist_matrix_filename.c_str(), "euclidean") == 0) {
        new (this) Instance(cust_max_id, loc_max_id, weights_filename, capacities_filename, p, delim, type_service);
        return;
    }else{
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
            this->cust_max_id = cust_max_id;
            this->loc_max_id = loc_max_id;
            // Clear eof and fail flags, go to beginning
            dist_matrix_file.clear();
            dist_matrix_file.seekg(0);
            uint_t size = (loc_max_id + 1) * (cust_max_id + 1);
            cout << "Distance matrix dimensions: " << loc_max_id + 1 << " x " << cust_max_id + 1 << " = " << size << "\n";
            tock(start);
            // Load weights
            start = tick();
            cout << "Loading weights...\n";
            total_demand = 0.0;
            cust_weights = shared_ptr<dist_t[]>(new dist_t[cust_max_id + 1], std::default_delete<dist_t[]>());
            for (uint_t cust = 0; cust < cust_max_id + 1; cust++) cust_weights[cust] = DEFAULT_WEIGHT;
            // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
            cust_coordinates.assign(cust_max_id + 1, std::make_pair(0, 0));
            getline(weights_file, line); // skip first line
            cout << "Skipped line: " << line << endl;
            uint_t w_cnt = 0;
            dist_t max_weight = 0;
            dist_t min_weight = std::numeric_limits<dist_t>::max();
            dist_t sum_sq_weight = 0; // sum of squared weights
            while (getline(weights_file, line)) {
                auto tokens = tokenize(line, delim);
                auto cust = stoi(tokens[0]);
                auto weight = stod(tokens[1]);
                cust_weights[cust] = weight;
                total_demand += weight;
                w_cnt++;
                if (weight > max_weight) max_weight = weight;
                if (weight < min_weight) min_weight = weight;
                sum_sq_weight += weight * weight; // Adding squared weight
            }
            cout << "Loaded " << w_cnt << " weights\n";
            cout << "Total customer demand: " << fixed << setprecision(2) << total_demand << endl;
            cout << "Max weight: " << max_weight << endl;
            cout << "Min weight: " << min_weight << endl;
            dist_t mean_weight = total_demand / w_cnt;
            dist_t variance_weight = sum_sq_weight / w_cnt - mean_weight * mean_weight;
            dist_t stdev_weight = sqrt(variance_weight);
            cout << "Mean: " << mean_weight << endl;
            cout << "dists stdev: " << stdev_weight << endl;
            tock(start);
            // Load capacities
            start = tick();
            cout << "Loading capacities...\n";
            loc_capacities = shared_ptr<dist_t[]>(new dist_t[loc_max_id + 1], std::default_delete<dist_t[]>());
            for (uint_t loc = 0; loc < loc_max_id + 1; loc++) loc_capacities[loc] = DEFAULT_CAPACITY;
            // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
            loc_coordinates.assign(loc_max_id + 1, std::make_pair(0, 0));
            getline(capacities_file, line); // skip first line
            cout << "Skipped line: " << line << endl;
            uint_t cap_cnt = 0;
            dist_t max_cap = 0;
            dist_t min_cap = std::numeric_limits<dist_t>::max();
            dist_t sum_sq_cap = 0; // sum of squared distances
            total_capacity = 0.0;
            while (getline(capacities_file, line)) {
                auto tokens = tokenize(line, delim);
                auto loc = stoi(tokens[0]);
                auto cap = stod(tokens[1]);
                loc_capacities[loc] = cap;
                total_capacity += cap;
                cap_cnt++;
                if (cap > max_cap) max_cap = cap;
                if (cap < min_cap) min_cap = cap;
                sum_sq_cap += cap * cap; // Adding squared cap

            }
            cout << "Loaded " << cap_cnt << " capacities\n";
            cout << "Total capacity: " << fixed << setprecision(2) << total_capacity << endl;
            cout << "Max capacity: " << max_cap << endl;
            cout << "Min capacity: " << min_cap << endl;
            dist_t mean_cap = total_capacity / cap_cnt;
            dist_t variance_cap = sum_sq_cap / cap_cnt - mean_cap * mean_cap;
            dist_t stdev_cap = sqrt(variance_cap);
            cout << "Mean: " << mean_cap << endl;
            cout << "dists stdev: " << stdev_cap << endl;
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
            dist_t max_dist = 0;
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
                if (dist > max_dist) max_dist = dist;
                sum_sq += dist * dist;
                cnt++;
            }
            cout << "Max dist: " << max_dist << endl;
            // Determine stdev and bandwidth
            calculate_Bandwidth(sum, sum_sq, cnt);


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
}

// Instane construtor and dist matrix with euclidean distances
Instance::Instance(uint_t cust_max_id, uint_t loc_max_id, const string &weights_filename, const string &capacities_filename, uint_t p, char delim, string type_service) : p(p), type_service(type_service) {

    this->cust_max_id = cust_max_id;
    this->loc_max_id = loc_max_id;

    // Open streams
    fstream weights_file(weights_filename);
    fstream capacities_file(capacities_filename);
    //
    if (weights_file.is_open() && capacities_file.is_open()) {
        if (loc_max_id == 0 || cust_max_id == 0) {
            cerr << "Error: loc_max_id or cust_max_id not set\n";
            exit(-1);
        }
        string line;
        // Scan data to determine distance matrix dimensions
        cout << "Scanning input data...\n";
        auto start = tick();
        uint_t size = (loc_max_id + 1) * (cust_max_id + 1);
        cout << "Distance matrix dimensions: " << loc_max_id + 1 << " x " << cust_max_id + 1 << " = " << size << "\n";
        tock(start);
        // Load weights
        start = tick();
        cout << "Loading weights...\n";
        total_demand = 0.0;
        cust_weights = shared_ptr<dist_t[]>(new dist_t[cust_max_id + 1], std::default_delete<dist_t[]>());
        for (uint_t cust = 0; cust < cust_max_id + 1; cust++) cust_weights[cust] = DEFAULT_WEIGHT;
        // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
        cust_coordinates.assign(cust_max_id + 1, std::make_pair(0, 0));
        getline(weights_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t w_cnt = 0;
        uint_t coord_cnt = 0;
        dist_t max_weight = 0;
        dist_t min_weight = std::numeric_limits<dist_t>::max();
        dist_t sum_sq_weight = 0; // sum of squared weights
        while (getline(weights_file, line)) {
            auto tokens = tokenize(line, delim);
            auto cust = stoi(tokens[0]);
            auto weight = stod(tokens[1]);
            cust_weights[cust] = weight;
            total_demand += weight;
            w_cnt++;
            if (weight > max_weight) max_weight = weight;
            if (weight < min_weight) min_weight = weight;
            sum_sq_weight += weight * weight; // Adding squared weight
            if (tokens.size() >= 4) {
                auto x_coord = stod(tokens[2]);
                auto y_coord = stod(tokens[3]);
                cust_coordinates[cust] = std::make_pair(x_coord, y_coord);
                coord_cnt++;
            }
        }
        cout << "Loaded " << w_cnt << " weights\n";
        if (coord_cnt > 0) cout << "Loaded " << coord_cnt << " coordinates\n";
        cout << "Total customer demand: " << fixed << setprecision(2) << total_demand << endl;
        cout << "Max weight: " << max_weight << endl;
        cout << "Min weight: " << min_weight << endl;
        dist_t mean_weight = total_demand / w_cnt;
        dist_t variance_weight = sum_sq_weight / w_cnt - mean_weight * mean_weight;
        dist_t stdev_weight = sqrt(variance_weight);
        cout << "Mean: " << mean_weight << endl;
        cout << "dists stdev: " << stdev_weight << endl;
        tock(start);
        // Load capacities
        start = tick();
        cout << "Loading capacities...\n";
        loc_capacities = shared_ptr<dist_t[]>(new dist_t[loc_max_id + 1], std::default_delete<dist_t[]>());
        for (uint_t loc = 0; loc < loc_max_id + 1; loc++) loc_capacities[loc] = DEFAULT_CAPACITY;
        // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
        loc_coordinates.assign(loc_max_id + 1, std::make_pair(0, 0));
        getline(capacities_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t cap_cnt = 0;
        coord_cnt = 0;
        total_capacity = 0.0;
        dist_t max_cap = 0;
        dist_t min_cap = std::numeric_limits<dist_t>::max();
        dist_t sum_sq_cap = 0; // sum of squared capacities
        while (getline(capacities_file, line)) {
            auto tokens = tokenize(line, delim);
            auto loc = stoi(tokens[0]);
            auto cap = stod(tokens[1]);
            loc_capacities[loc] = cap;
            cap_cnt++;
            total_capacity += cap;
            if (cap > max_cap) max_cap = cap;
            if (cap < min_cap) min_cap = cap;
            sum_sq_cap += cap * cap; // Adding squared distance twice

            if (tokens.size() >= 4) {
                auto x_coord = stod(tokens[2]);
                auto y_coord = stod(tokens[3]);
                loc_coordinates[loc] = std::make_pair(x_coord, y_coord);
                coord_cnt++;
            }
        }
        cout << "Loaded " << cap_cnt << " capacities\n";
        if (coord_cnt > 0) cout << "Loaded " << coord_cnt << " coordinates\n";
        cout << "Total capacity: " << fixed << setprecision(2) << total_capacity << endl;
        cout << "Max capacity: " << max_cap << endl;
        cout << "Min capacity: " << min_cap << endl;
        dist_t mean_cap = total_capacity / cap_cnt;
        dist_t variance_cap = sum_sq_cap / cap_cnt - mean_cap * mean_cap;
        dist_t stdev_cap = sqrt(variance_cap);
        cout << "Mean: " << mean_cap << endl;
        cout << "dists stdev: " << stdev_cap << endl;
        tock(start);

        // dist matrix using euclidean distances
        start = tick();
        cout << "Loading distance matrix...\n";
        dist_matrix = shared_ptr<dist_t[]>(new dist_t[size], std::default_delete<dist_t[]>());
        for (uint_t i = 0; i < size; i++) {
            dist_matrix[i] = DEFAULT_DISTANCE;
        }
        vector<bool> loc_flags(loc_max_id + 1, false);
        vector<bool> cust_flags(cust_max_id + 1, false);
        // Fill it
        cout << "Computing euclidean distances...\n";
        dist_t sum = 0; // sum of distances
        dist_t sum_sq = 0; // sum of squared distances
        dist_t max_dist = 0;
        uint_t cnt = 0;
        // Loop through all pairs of locations and customers

        for (uint_t i = 1; i < cust_max_id+1; i++) {
            for (uint_t j = i; j < loc_max_id+1; j++) {
                if (i >= cust_coordinates.size() || j >= loc_coordinates.size()) {
                    cerr << "Error: loc or cust coordinates not found\n";
                    exit(-1);
                } else {
                    uint_t cust =i;
                    uint_t loc = j;
                    auto x_diff = loc_coordinates[loc].first - cust_coordinates[cust].first;
                    auto y_diff = loc_coordinates[loc].second - cust_coordinates[cust].second;
                    dist_t euclidean_dist = sqrt(x_diff * x_diff + y_diff * y_diff);
                    // Round euclidean_dist to two decimal places
                    // euclidean_dist = round(euclidean_dist * 1000) / 1000; // Rounds to two decimal places

                    setDist(loc, cust, euclidean_dist);
                    setDist(cust, loc, euclidean_dist); // Set symmetric distance
                    loc_flags[loc] = true;
                    loc_flags[cust] = true; // Set flags for both loc and cust
                    cust_flags[cust] = true;
                    cust_flags[loc] = true; // Set flags for both cust and loc
                    sum += euclidean_dist * 2; // Adding twice the distance since it's symmetric
                    if (euclidean_dist > max_dist) max_dist = euclidean_dist;
                    sum_sq += euclidean_dist * euclidean_dist * 2; // Adding squared distance twice
                    cnt += 2; // Incrementing count by 2 since each distance is added twice
                }
            }
        }
        cout << "Max dist: " << max_dist << endl;
        // Determine stdev and bandwidth
        calculate_Bandwidth(sum, sum_sq, cnt);

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

        if(!weights_file) cerr << "- Weights file" << endl;
        if(!capacities_file) cerr << "- Capacities file" << endl;
        
        cerr << "Check if the path is correct and/or the correct name was given for the concerned files." << endl;
        exit(-1);

    }

}

void Instance::calculate_Bandwidth(dist_t sum, dist_t sum_sq, uint_t cnt) {
    dist_t mean = sum / cnt;
    dist_t variance = sum_sq / cnt - mean * mean;
    dist_t stdev = sqrt(variance);
    h = pow((4 * pow(stdev, 5)) / (3 * cnt), 0.2);

    cout << "bw multiplier: " << BW_MULTIPLIER << endl;
    h = BW_MULTIPLIER * h;
    cout << "Loaded " << cnt << " distances\n";
    cout << "dists stdev: " << stdev << endl;
    cout << "bandwidth h: " << h << endl;
}

void Instance::set_isBinProblem(bool isBinProblem) {
    this->isBinProblem = isBinProblem;
}


bool Instance::get_isWeightedObjFunc() {
    return is_weighted_obj_func;
}
void Instance::set_isWeightedObjFunc(bool isWeightedObjFunc) {
    is_weighted_obj_func = isWeightedObjFunc;
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

void Instance::setVotedLocs(vector<uint_t> voted_locs) {
    this->voted_locs = voted_locs;
}
vector<uint_t> Instance::getVotedLocs() {
    return this->voted_locs;
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

    if (cover_mode_n2){
        cout << "cover_mode_n2" << endl;
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea, unique_subareas_n2, loc_coverages_n2, type_subarea_n2);
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
    if (get_isWeightedObjFunc()){
        cout << "weighted obj function" << endl;
    } else {
        cout << "unweighted obj function" << endl;
    }
    if (cover_mode) {
        cout << "cover_max_id: " << cover_max_id << endl;
        cout << "subareas_cnt: " << unique_subareas.size() << endl;
        cout << "subareas: "<< type_subarea << endl;
    }
    if (cover_mode_n2) {
        cout << "cover_max_id_n2: " << cover_n2_max_id << endl;
        cout << "subareas_n2 cnt: " << unique_subareas_n2.size() << endl;
        cout << "subareas n2: "<< type_subarea_n2 << endl;
    }
    cout << "p: " << p << endl;
    cout << "total_demand: " << fixed << setprecision(2) << total_demand << endl;
    if(threshold_dist > 0){
        cout << "threshold_dist: " << threshold_dist << endl;
    }
    cout << endl << endl;

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

uint_t  Instance::getClosestLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    uint_t loc_cl = numeric_limits<uint_t>::max();
    for (auto loc:locations) {
        auto dist = getRealDist(loc, cust);
        if (dist <= dist_min) {
            dist_min = dist;
            loc_cl = loc;
        }
    }
    // cout <<"dist_min: " << dist_min << endl;
    return loc_cl;
}

uint_t Instance::getClosestLoc_notloc(uint_t cust, uint_t loc_avoid) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    uint_t loc_cl = numeric_limits<uint_t>::max();
    for (auto loc:locations) {
        auto dist = getRealDist(loc, cust);
        if (dist <= dist_min && loc != loc_avoid) {
            dist_min = dist;
            loc_cl = loc;
        }
    }
    // cout <<"dist_min: " << dist_min << endl;
    return loc_cl;
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
    // return Instance(locations_new, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id, type_service);

    if (!cover_mode) {
        return Instance(locations_new, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id,type_service);
    }

    if (cover_mode_n2){
        return Instance(locations_new, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea, unique_subareas_n2, loc_coverages_n2, type_subarea_n2);
    }

    return Instance(locations_new, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);


}

dist_t Instance::getLocCapacity(uint_t loc) {
    return loc_capacities[loc];
}

dist_t Instance::getCustDemand(uint_t cust) {
    return cust_weights[cust];
}

dist_t Instance::getTotalDemand() const {
    return total_demand;
}

string Instance::getTypeService() const {
    return type_service;
}

string Instance::getTypeSubarea_n2() const {
    return type_subarea_n2;
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
        cout << "Error] while trying to open the subareas file" << endl;
        cout << "subareas filename: " << coverages_filename << endl;
    }
}

void Instance::ReadCoverages_n2(const string& coverages_filename,string type_subarea_n2,char delim){
    this->type_subarea_n2 = type_subarea_n2;
    fstream coverages_file(coverages_filename);
    cover_n2_max_id = 0;
    if (coverages_file.is_open()) {
        string line;
        // Load coverages
        auto start = tick();
        cout << "Loading coverages...\n";
        loc_coverages_n2 = shared_ptr<uint_t[]>(new uint_t[loc_max_id + 1], default_delete<uint_t[]>());
        for (uint_t loc = 0; loc < loc_max_id + 1; loc++) loc_coverages_n2[loc] = 0;
        getline(coverages_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t cover_cnt = 0; 
        vector<uint_t> qtd_coverages; // to store the number of coverages per subarea
        while (getline(coverages_file, line)) {
            auto tokens = tokenize(line, delim);
            auto loc = stoi(tokens[0]);
            auto subarea = stoi(tokens[1]);
            cover_n2_max_id = max(cover_n2_max_id, (uint_t) subarea);
            loc_coverages_n2[loc] = subarea;
            unique_subareas_n2.insert(subarea);
            cover_cnt++;
        }
        cout << "Loaded " << cover_cnt << " locations covered\n";
        cout << "Number of subareas: " << unique_subareas_n2.size() << endl;

        // Sttistics about the number of coverages per subarea
        for (auto subarea:unique_subareas_n2) {
            // cout << "Subarea " << subarea << " has " << count(loc_coverages.get(), loc_coverages.get() + loc_max_id + 1, subarea) << " locations" << endl;
            qtd_coverages.push_back(count(loc_coverages_n2.get(), loc_coverages_n2.get() + loc_max_id + 1, subarea));
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

uint_t Instance::getSubareaLocation_n2(uint_t loc){
    return loc_coverages_n2[loc];
}

unordered_set<uint_t> Instance::getSubareasSet(){
    return unique_subareas;
}

unordered_set<uint_t> Instance::getSubareasSet_n2(){
    return unique_subareas_n2;
}

const vector<uint_t> Instance::getLocationsSubarea(uint_t subarea){
    vector<uint_t> vet;
    for(uint_t loc = 0; loc < loc_max_id + 1; loc++){
        if(loc_coverages[loc] == subarea)
            vet.push_back(loc);
    }
    return vet;
}

const vector<uint_t> Instance::getLocationsSubarea_n2(uint_t subarea){
    vector<uint_t> vet;
    for(uint_t loc = 0; loc < loc_max_id + 1; loc++){
        if(loc_coverages_n2[loc] == subarea)
            vet.push_back(loc);
    }
    return vet;
}


bool Instance::isInTheSameSubarea(uint_t loc1, uint_t loc2){
    return loc_coverages[loc1] == loc_coverages[loc2];
}

bool Instance::isInTheSameSubarea_n2(uint_t loc1, uint_t loc2){
    return loc_coverages_n2[loc1] == loc_coverages_n2[loc2];
}

void Instance::setCoverModel(bool cover_mode){
    this->cover_mode = cover_mode;
}

void Instance::setCoverModel_n2(bool cover_mode_n2){
    this->cover_mode_n2 = cover_mode_n2;
}

bool Instance::isCoverMode() {
    return cover_mode;
}
bool Instance::isCoverMode_n2() {
    return cover_mode_n2;
}

bool Instance::isPcoversAllSubareas(unordered_set<uint_t> p_loc_cand){

    bool verb = false;


    if (p_loc_cand.size() != p) {
        if (verb) cout << "ERROR: number of locations is different from p" << endl;
        return false;
    }

    int num_subareas = unique_subareas.size();

    if (p_loc_cand.size() >= static_cast<std::unordered_set<unsigned int>::size_type>(num_subareas)) {
        for(auto subarea:unique_subareas){
            auto loc_subarea = getLocationsSubarea(subarea);
            bool covered = false;
            for(auto loc:loc_subarea){
                if (p_loc_cand.find(loc) != p_loc_cand.end()){
                    covered = true;
                }
            }
            if (!covered){
                if (verb) cout << "ERROR: subarea not covered" << endl;
                if (verb) cout << "subarea: " << subarea << endl;
                return false;
            }

        }
    }else{

        int cont_cover=0;
        for(auto subarea:unique_subareas){
            auto loc_subarea = getLocationsSubarea(subarea);
            for(auto loc:loc_subarea){
                if (p_loc_cand.find(loc) != p_loc_cand.end()){
                    cont_cover++;
                    break;
                }
            }
        }

        if (verb) cout << "Number of subareas covered: " << cont_cover << " of " << num_subareas << endl;
        if (verb) cout << "Number of locations: " << p_loc_cand.size() << endl;
        // if (cont_cover < std::min(p_loc_cand.size(), static_cast<std::unordered_set<unsigned int>::<size_type>(num_subareas))){ 
        if (static_cast<std::unordered_set<unsigned int>::size_type>(cont_cover) < p) {
            if (verb) cout << "ERROR: max number of subareas not covered" << endl;
            return false;
        }

    }

    return true;
}

bool Instance::isPcoversAllSubareas_n2(unordered_set<uint_t> p_loc_cand){

    bool verb = false;
    if (p_loc_cand.size() != p) {
        if (verb) cout << "ERROR: number of locations is different from p" << endl;
        return false;
    }
    int num_subareas = unique_subareas_n2.size();

    if (p_loc_cand.size() >= static_cast<std::unordered_set<unsigned int>::size_type>(num_subareas)) {
        for(auto subarea:unique_subareas_n2){
            auto loc_subarea = getLocationsSubarea_n2(subarea);
            bool covered = false;
            for(auto loc:loc_subarea){
                if (p_loc_cand.find(loc) != p_loc_cand.end()){
                    covered = true;
                }
            }
            if (!covered){
                if (verb) cout << "ERROR: subarea_n2 not covered" << endl;
                if (verb) cout << "subarea_n2: " << subarea << endl;
                if (verb) cout << "type subarea n2: " << type_subarea_n2 << endl;
                return false;
            }

        }
    }else{

        int cont_cover=0;
        for(auto subarea:unique_subareas_n2){
            auto loc_subarea = getLocationsSubarea_n2(subarea);
            for(auto loc:loc_subarea){
                if (p_loc_cand.find(loc) != p_loc_cand.end()){
                    cont_cover++;
                    break;
                }
            }
        }

        if (verb) cout << "Number of subareas covered: " << cont_cover << " of " << num_subareas << endl;
        if (verb) cout << "Number of locations: " << p_loc_cand.size() << endl;
        // if (cont_cover < std::min(p_loc_cand.size(), static_cast<std::unordered_set<unsigned int>::<size_type>(num_subareas))){ 
        if (static_cast<std::unordered_set<unsigned int>::size_type>(cont_cover) < p) {
            if (verb) cout << "ERROR: max number of subareas not covered" << endl;
            return false;
        }

    }

    return true;

}

uint_t Instance::getLocIndex(uint_t loc){

    auto it = find(locations.begin(), locations.end(), loc);
    // Check if element was found and get its index
    uint_t index=10000000; // UNINT_MAX
    if (it != locations.end()) {
        index = distance(locations.begin(), it); // Calculate index using iterator
        // cout << "Index of " << loc << " is: " << index << endl;
    } else {
        cout << "Element not found in vector." << endl;
    }

    return index;
}

uint_t Instance::getCustIndex(uint_t cust){

    auto it = find(customers.begin(), customers.end(), cust);
    // Check if element was found and get its index
    uint_t index=10000000; // UNINT_MAX
    if (it != customers.end()) {
        index = distance(customers.begin(), it); // Calculate index using iterator
        // cout << "Index of " << cust << " is: " << index << endl;
    } else {
        cout << "Element not found in vector." << endl;
    }

    return index;
    
}

Instance Instance::filterInstance(string type_service) {

    // remove all locations with capacity less or equal than 1
    vector<uint_t> locations_filtered;
    for (auto loc:locations) {
        if (loc_capacities[loc] > 1) {
            locations_filtered.push_back(loc);
        }
    }
    
    // getReducedSubproblem(locations_filtered, type_service);
    if (!cover_mode) {
        return Instance(locations_filtered, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id,type_service);
    }

    if (cover_mode_n2){
        return Instance(locations_filtered, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea, unique_subareas_n2, loc_coverages_n2, type_subarea_n2);
    }

    return Instance(locations_filtered, customers, cust_weights, loc_capacities, dist_matrix, p, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);
}

void Instance::set_ThresholdDist(dist_t threshold_dist){
    this->threshold_dist = threshold_dist;
}
dist_t Instance::get_ThresholdDist(){
    return threshold_dist;
}

void Instance::set_MaxLimitAssignments(uint_t max_limit_assignments){
    this->max_limit_assignments = max_limit_assignments;
}

uint_t Instance::get_MaxLimitAssignments(){
    return max_limit_assignments;
}


vector<uint_t> Instance::get_kClosestLocations(uint_t loc, uint_t k) {
    
    // Validate input
    if (loc > locations.size()) { // attention: loc is the index of the location in the vector locations values = 1 ... loc_size  not start from 0
        cerr << "Error: Invalid location index. (kclosest locs)" << endl;
        cerr << "returning empty vector" << endl;
        return {};
    }

    // Create a vector to store distances and indices
    vector<pair<dist_t, uint_t>> dist_index;

    // Compute distances from location loc to all other locations
    for (uint_t i = 0; i < locations.size(); ++i) {
        if (i != loc) {
            dist_t dist = getRealDist(loc, i);
            if(get_isWeightedObjFunc()) dist = getWeightedDist(loc, i);
            dist_index.emplace_back(dist, i);
        }
    }

    // Sort distances in ascending order
    sort(dist_index.begin(), dist_index.end());

    // Extract the indices of the k closest locations
    vector<uint_t> closest_locations;
    for (uint_t i = 0; i < k && i < dist_index.size(); ++i) {
        closest_locations.push_back(dist_index[i].second);
    }


    // print the 5 lowest distances
    // for (uint_t i = 0; i < 5; ++i) {
    //     cout << "dist loc " << loc << " to " << dist_index[i].second << " = " << dist_index[i].first << endl;
    // }
    // print the k closest locations
    // cout << "k closest locations to " << loc << " are: ";
    // for (auto loc_close:closest_locations) cout << loc_close << "(" << std::fixed << std::setprecision(2) << getRealDist(loc,loc_close) << ") "<< " coord: " << loc_coordinates[loc].first << "," << loc_coordinates[loc].second << " | ";


    return closest_locations;
}

vector<uint_t> Instance::get_kClosestLocations_notin_plocs(uint_t loc, uint_t k, unordered_set<uint_t> p_locs) {
    // Validate input
    if (loc > locations.size()) { // attention: loc is the index of the location in the vector locations values = 1 ... loc_size  not start from 0
        cerr << "Error: Invalid location index. (kclosest locs notin plocs)" << endl;
        cerr << "returning empty vector" << endl;
        return {};
    }

    // Create a vector to store distances and indices
    vector<pair<dist_t, uint_t>> dist_index;

    // Compute distances from location loc to all other locations
    for (uint_t i = 0; i < locations.size(); ++i) {
        if (i != loc && p_locs.find(i) == p_locs.end()) {
            dist_t dist = getRealDist(loc, i);
            if(get_isWeightedObjFunc()) dist = getWeightedDist(loc, i);
            dist_index.emplace_back(dist, i);
        }
    }

    // Sort distances in ascending order
    sort(dist_index.begin(), dist_index.end());

    // Extract the indices of the k closest locations
    vector<uint_t> closest_locations;
    for (uint_t i = 0; i < k && i < dist_index.size(); ++i) {
        closest_locations.push_back(dist_index[i].second);
    }

    return closest_locations;

}




// Function to initialize the centroids by selecting random points from the dataset
vector<uint_t> Instance::initializeCentroids(uint_t n, uint_t k, uint_t seed) {
    // seed generator
    std::vector<uint_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0); // Fill indices with 0, 1, 2, ..., n-1

    std::mt19937 rng(seed);
    std::shuffle(indices.begin(), indices.end(), rng); // Shuffle indices randomly

    std::vector<uint_t> centroids(indices.begin(), indices.begin() + k); // Take the first k shuffled indices

    return centroids;
}

// Function to assign each data point to the nearest centroid using the precomputed distance matrix
std::vector<uint_t> Instance::assignClusters(const std::shared_ptr<dist_t[]>& dist_matrix, const std::vector<uint_t>& centroids, uint_t n, int k) {
    std::vector<uint_t> clusters(n);
    for (uint_t i = 0; i < n; ++i) {
        dist_t minDist = std::numeric_limits<dist_t>::max();
        for (uint_t j = 0; j < k; ++j) {
            // double dist = getDistance(dist_matrix, i, centroids[j], n);
            dist_t dist = getRealDist(i, centroids[j]);
            if (dist < minDist) {
                minDist = dist;
                clusters[i] = j;
            }
        }
    }
    return clusters;
}

// Function to update the centroids
std::vector<uint_t> Instance::updateCentroids(const std::vector<uint_t>& clusters, const std::shared_ptr<dist_t[]>& dist_matrix, uint_t n, int k) {
    std::vector<uint_t> newCentroids(k);
    std::vector<dist_t> minSumDist(k, std::numeric_limits<dist_t>::max());

    for (uint_t i = 0; i < k; ++i) {
        for (uint_t j = 0; j < n; ++j) {
            if (clusters[j] == i) {
                dist_t sumDist = 0;
                for (uint_t l = 0; l < n; ++l) {
                    if (clusters[l] == i) {
                        // sumDist += getDistance(dist_matrix, j, l, n);
                        sumDist += getRealDist(j, l);
                    }
                }
                if (sumDist < minSumDist[i]) {
                    minSumDist[i] = sumDist;
                    newCentroids[i] = j;
                }
            }
        }
    }

    return newCentroids;
}

// K-Means algorithm using precomputed distance matrix
vector<uint_t> Instance::kMeans(const std::shared_ptr<dist_t[]>& dist_matrix, uint_t n, int k, uint_t seed, int maxIterations=100) {
    
    // seed generator
    std::mt19937 rng(seed); // Use Mersenne Twister for randomness
    // Step 1: Initialize centroids
    std::vector<uint_t> centroids = initializeCentroids(n, k, seed);

    std::vector<uint_t> clusters(n);
    for (int iter = 0; iter < maxIterations; ++iter) {
        // Step 2: Assign clusters
        clusters = assignClusters(dist_matrix, centroids, n, k);

        // Step 3: Update centroids
        std::vector<uint_t> newCentroids = updateCentroids(clusters, dist_matrix, n, k);

        // Check for convergence (if centroids don't change, stop)
        // if (newCentroids == centroids) break;
        // Check for convergence
        if (newCentroids == centroids) {
            std::cout << "Converged after " << iter + 1 << " iterations." << std::endl;
            break;
        }
        centroids = newCentroids;
    }

    return clusters;
}

void Instance::createClustersLocsWithKmeans(uint_t k, uint_t seed) {


    // check dist_matrix
    if (dist_matrix == nullptr) {
        cerr << "[Error]: no distance matrix found" << endl;
        return;
    }
    // check if the data have coordinates
    if (loc_coordinates.size() == 0 || cust_coordinates.size() == 0) {  
        cerr << "[Error]: no coordinates found for locations" << endl;
        return;
    }
    // check if the number of locations is less than the number of clusters
    if (locations.size() < k) {
        cerr << "[Error]: number of locations is less than the number of clusters" << endl;
        return;
    }


    std::vector<uint_t> clusters = kMeans(dist_matrix, uint_t(locations.size()), k, seed);

    // print number of locs in each cluster     
    std::vector<std::vector<uint_t>> clusters_locs(k);
    for (uint_t i = 0; i < clusters.size(); ++i) {
        clusters_locs[clusters[i]].push_back(i);
    }

    int sum = 0;
    int cont_empty = 0;
    for (uint_t i = 0; i < clusters_locs.size(); ++i) {
        // std::cout << "Cluster " << i << " has " << clusters_locs[i].size() << " locations" << std::endl;
        sum += clusters_locs[i].size();
        if (clusters_locs[i].size() == 0) {
            cont_empty++;
        }
    }
    if (cont_empty > 0) {
        cout << "[WARN] number of empty clusters: " << cont_empty << endl;
    }


    this->clusters_locs = clusters_locs;    

}

vector<vector<uint_t>>  Instance::getClusters(){
    return clusters_locs;
}

void Instance::setClusters(vector<vector<uint_t>> clusters_locs){

    vector<vector<uint_t>> clusters_locs_new;

    // check if the location exists in the instance if yes then add to clusters_locs_new
    int cont_locs = 0;
    for(auto cluster:clusters_locs){
        vector<uint_t> cluster_new;
        for(auto loc:cluster){
            if (find(locations.begin(), locations.end(), loc) != locations.end()) {
                cluster_new.push_back(loc);
                cont_locs++;
            }
        }
        clusters_locs_new.push_back(cluster_new);
    }

    cout << "Number of locations in clusters: " << cont_locs << endl;
 
    this->clusters_locs = clusters_locs_new;
}


Instance Instance::getSubproblemFromClusters(uint_t k){

    vector<uint_t> locations_new;
    vector<uint_t> customers_new;

    // check if clusters_locs is empty
    if (clusters_locs.size() == 0) {
        cerr << "[Error]: no clusters found" << endl;
        uint_t p_new = 0;
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);
    }

    // check if the number of clusters is less than the number of clusters
    if (clusters_locs.size() < k) {
        cerr << "[Error]: number of clusters is less than the number of cluster" << endl;
        uint_t p_new = 0;
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);
    }


    // CREATE locations_new and customers_new with fixed kth cluster
    for(auto loc:clusters_locs[k]){
        locations_new.push_back(locations[loc]);
        customers_new.push_back(customers[loc]);    
    }

    // check if sets have the same elements
    for(auto loc:locations_new){
        if (find(customers_new.begin(), customers_new.end(), loc) == customers_new.end()) {
            cerr << "[Error]: locations_new and customers_new have different elements" << endl;
            exit(-1);   
        }
    }

    // set p_new with percentage of number of locations
    // uint_t p_new = (locations_new.size() * p) / locations.size();
    uint_t p_new = ceil(locations_new.size() * 0.1);

    // print info susbproblem
    // cout << "Subproblem with cluster " << k << endl;
    // cout << "locations_new: " << locations_new.size() << endl;
    // cout << "customers_new: " << customers_new.size() << endl;
    // cout << "p_new: " << p_new << endl;


    if (!cover_mode) {
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service);
    }

    if (cover_mode_n2){
        cout << "cover_mode_n2" << endl;
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea, unique_subareas_n2, loc_coverages_n2, type_subarea_n2);
    }

    return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);
}

Instance Instance::sampleSubproblemFromClusters(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, uint_t num_clusters, uint_t seed){

    // generate random points proporcional to the number of locations in each cluster
    std::default_random_engine generator(seed);
    // seed generator
    std::mt19937 rng(seed); // Use Mersenne Twister for randomness
    std::vector<uint_t> locations_new;
    std::vector<uint_t> customers_new;

    createClustersLocsWithKmeans(num_clusters, seed);

    // check if clusters_locs is empty
    if (clusters_locs.size() == 0) {
        cout << "[Error] No clusters found during sampling" << endl;
        cout << "[Warning] Returning a random sample" << endl;
        return sampleSubproblem(loc_cnt, cust_cnt, p_new, seed);
    }

    // if cust_cnt is equal to the number of customers, then copy all customers
    if (cust_cnt == customers.size()) {
        customers_new = customers;
    } 

    if (loc_cnt == locations.size()) {
        locations_new = locations;
    }

    // Total number of elements across all clusters
    uint_t total_elements = 0;
    for (const auto& cluster : clusters_locs) {
        total_elements += cluster.size();
    }

    // Check if there are enough elements
    if (total_elements < loc_cnt || total_elements < cust_cnt) {
        cout << "[Error] Not enough elements across all clusters to loc_cnt or cust_cnt." << std::endl;
        cout << "[Warning] Returning a random sample" << endl;
        return sampleSubproblem(loc_cnt, cust_cnt, p_new, seed);
    }



    // Track added elements to ensure uniqueness
    std::unordered_set<uint_t> added_elements;

    // Add elements until loc_cnt and cust_cnt are satisfied
    while (locations_new.size() < loc_cnt || customers_new.size() < cust_cnt) {
        // Calculate cluster weights
        std::vector<double> cluster_weights(clusters_locs.size(), 0.0);
        for (size_t i = 0; i < clusters_locs.size(); ++i) {
            if (!clusters_locs[i].empty()) {
                cluster_weights[i] = clusters_locs[i].size();
            }
        }

        // Create distribution
        std::discrete_distribution<int> distribution(cluster_weights.begin(), cluster_weights.end());

        // Pick a cluster and ensure it is not empty
        uint_t cluster;
        do {
            cluster = distribution(generator);
        } while (clusters_locs[cluster].empty());

        // Randomly select a location from the chosen cluster
        std::uniform_int_distribution<uint_t> distribution_loc(0, clusters_locs[cluster].size() - 1);
        uint_t loc = clusters_locs[cluster][distribution_loc(generator)];

        // Ensure that we have not already added this element
        if (added_elements.find(loc) == added_elements.end()) {
            locations_new.push_back(locations[loc]);
            added_elements.insert(loc);

            if (customers_new.size() < cust_cnt) { // if customers_subproblem is diff from all customers
                customers_new.push_back(customers[loc]);
            }
        }
    }

    if (!cover_mode) {
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service);
    }

    if (cover_mode_n2){
        cout << "cover_mode_n2" << endl;
        return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea, unique_subareas_n2, loc_coverages_n2, type_subarea_n2);
    }

    return Instance(locations_new, customers_new, cust_weights, loc_capacities, dist_matrix, p_new, loc_max_id, cust_max_id,type_service, unique_subareas, loc_coverages, type_subarea);

}