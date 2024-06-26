#include "instance.hpp"

#include <utility>
#include <sstream>
#include <string.h>


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
    cover_mode = true;
    total_demand = 0;
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
    total_demand = 0;
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


Instance::Instance(const string &dist_matrix_filename, const string &weights_filename, const string& capacities_filename, uint_t p, char delim, string type_service, uint_t cust_max_id, uint_t loc_max_id) : p(p), type_service(type_service) {

    if (strcmp(dist_matrix_filename.c_str(), "euclidian") == 0) {
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
            total_demand = 0;
            cust_weights = shared_ptr<dist_t[]>(new dist_t[cust_max_id + 1], std::default_delete<dist_t[]>());
            for (uint_t cust = 0; cust < cust_max_id + 1; cust++) cust_weights[cust] = DEFAULT_WEIGHT;
            // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
            cust_coordinates.assign(cust_max_id + 1, std::make_pair(0, 0));
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
            // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
            loc_coordinates.assign(loc_max_id + 1, std::make_pair(0, 0));
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


// Instane construtor and dist matrix with euclidian distances
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
        total_demand = 0;
        cust_weights = shared_ptr<dist_t[]>(new dist_t[cust_max_id + 1], std::default_delete<dist_t[]>());
        for (uint_t cust = 0; cust < cust_max_id + 1; cust++) cust_weights[cust] = DEFAULT_WEIGHT;
        // Predefine the size of cust_coordinates to match cust_weights and initialize with default value (0,0)
        cust_coordinates.assign(cust_max_id + 1, std::make_pair(0, 0));
        getline(weights_file, line); // skip first line
        cout << "Skipped line: " << line << endl;
        uint_t w_cnt = 0;
        uint_t coord_cnt = 0;
        while (getline(weights_file, line)) {
            auto tokens = tokenize(line, delim);
            auto cust = stoi(tokens[0]);
            auto weight = stod(tokens[1]);
            cust_weights[cust] = weight;
            total_demand += weight;
            w_cnt++;
            if (tokens.size() >= 4) {
                auto x_coord = stod(tokens[2]);
                auto y_coord = stod(tokens[3]);
                cust_coordinates[cust] = std::make_pair(x_coord, y_coord);
                coord_cnt++;
            }
        }
        cout << "Loaded " << w_cnt << " weights\n";
        if (coord_cnt > 0) cout << "Loaded " << coord_cnt << " coordinates\n";
        cout << "Total customer demand: " << total_demand << endl;
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
        while (getline(capacities_file, line)) {
            auto tokens = tokenize(line, delim);
            auto loc = stoi(tokens[0]);
            auto cap = stod(tokens[1]);
            loc_capacities[loc] = cap;
            cap_cnt++;

            if (tokens.size() >= 4) {
                auto x_coord = stod(tokens[2]);
                auto y_coord = stod(tokens[3]);
                loc_coordinates[loc] = std::make_pair(x_coord, y_coord);
                coord_cnt++;
            }
        }
        cout << "Loaded " << cap_cnt << " capacities\n";
        if (coord_cnt > 0) cout << "Loaded " << coord_cnt << " coordinates\n";
        tock(start);

        // dist matrix using euclidian distances
        start = tick();
        cout << "Loading distance matrix...\n";
        dist_matrix = shared_ptr<dist_t[]>(new dist_t[size], std::default_delete<dist_t[]>());
        for (uint_t i = 0; i < size; i++) {
            dist_matrix[i] = DEFAULT_DISTANCE;
        }
        vector<bool> loc_flags(loc_max_id + 1, false);
        vector<bool> cust_flags(cust_max_id + 1, false);
        // Fill it
        cout << "Computing euclidian distances...\n";
        dist_t sum = 0; // sum of distances
        dist_t sum_sq = 0; // sum of squared distances
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
                    sum_sq += euclidean_dist * euclidean_dist * 2; // Adding squared distance twice
                    cnt += 2; // Incrementing count by 2 since each distance is added twice
                }
            }
        }

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
    h = BW_MULTIPLIER * h;

    cout << "Loaded " << cnt << " distances\n";
    cout << "dists stdev: " << stdev << endl;
    cout << "bandwidth h: " << h << endl;
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
    cout << "total_demand: " << total_demand << endl;
    if(threshold_dist > 0){
        cout << "threshold_dist: " << threshold_dist << endl;
    }
    cout << endl << endl;

    // print cust, loc, dist value for ten pairs
    // for (uint_t i = 0; i < 4; i++) {
    //     auto cust = customers[i];
    //     for (uint_t j = 0; j < 4; j++) {
    //         auto loc = locations[j];
    //         cout << cust << " " << loc << " " << getRealDist(locations[j], customers[i]) << endl;
    //     }
    // }
    // for (auto cust:customers) {
    //     for (auto loc:locations) {
    //         cout << cust << " " << loc << " " << getRealDist(loc, cust) << endl;
    //     }
    // }


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