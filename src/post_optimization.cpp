#include "post_optimization.hpp"



// Constructor definitions
PostOptimization::PostOptimization(const std::shared_ptr<Instance>& instance, const Config& config, Solution_cap solution) 
    : config(config), is_cap(true), instance(instance), solution_cap(std::move(solution)) {
    std::cout << "PostOptimization constructor for Solution_cap" << std::endl;


    std::string methodToCheck;
    if (config.Method == "RSSV") {
        methodToCheck = config.Method_RSSV_fp;
    } else {
        methodToCheck = config.Method;
    }

    if (methodToCheck.size() >= 4 && methodToCheck.compare(methodToCheck.size() - 4, 4, "_BIN") == 0) {
        setBinModel(true);
    }



    // this->solution_cap.print();
}

PostOptimization::PostOptimization(const std::shared_ptr<Instance>& instance, const Config& config, Solution_std solution) 
    : config(config), is_cap(false), instance(instance), solution_std(std::move(solution)) {
    std::cout << "PostOptimization constructor for Solution_std" << std::endl;

    // this->solution_std.print();
}

// Destructor definition
PostOptimization::~PostOptimization() {
    // Destructor implementation (if any)
    std::cout << "PostOptimization destructor" << std::endl;
}

void PostOptimization::createSelectedLocations(int num_k) {
    cout << "Creating selected locations" << endl;
    
    // vector<uint_t> selectedLocations;
    auto p_locations = this->solution_cap.get_pLocations();


    selectedLocations.clear();

    for (auto p : p_locations) {
        // get k locs more close to p
        auto k_locs = this->instance->get_kClosestLocations(p, num_k);

        // add k locations
        selectedLocations.insert(selectedLocations.end(), k_locs.begin(), k_locs.end());

        // add only the kth position
        // uint_t lastValue = k_locs.back();
        // selectedLocations.push_back(lastValue);

    }

    // add p locations
    selectedLocations.insert(selectedLocations.end(), p_locations.begin(), p_locations.end());

    // // clean duplicates
    sort(selectedLocations.begin(), selectedLocations.end());
    selectedLocations.erase(unique(selectedLocations.begin(), selectedLocations.end()), selectedLocations.end());

    cout << endl << endl;
    cout << "Post-Optimization Selected " << selectedLocations.size() <<" Locations: ";
    for (auto loc : selectedLocations) {
        cout << loc << " ";
    }
    cout << endl << endl;
    cout << "Selected locations size: " << selectedLocations.size() << endl;

}



void PostOptimization::run(string Method_name) {
    cout << "\n\n[INFO] Running post-optimization" << endl;
    cout << "Time limit: " << timelimit << endl;
    cout << "Method: " << Method_name << endl;
    cout << "Is Bin Model: " << isBinModel << endl;

    uint_t p_value = uint_t(this->instance->get_p());
    auto num_facilities = this->instance->getLocations().size();
    double alpha = 0.5;
    bool finish = false;
    int iter = 1;


    while (!finish) {
        // Calculate the number of facilities for post-optimization in this iteration
        uint_t num_facilities_added = ceil(alpha * p_value);
        uint_t num_facilities_po = p_value + num_facilities_added;

        printIterationInfo(iter, num_facilities_po, alpha);
        auto start = std::chrono::high_resolution_clock::now();

        // Check if we are done and need to finish
        if (num_facilities_po >= num_facilities) {
            finish = true;
            cout << "[INFO] Num facilities is equal to or exceeds the original instance. Final iteration with all facilities." << endl;
            num_facilities_po = num_facilities;
        }

        Solution_cap solution_curr = this->solution_cap;
        solution_curr.statsDistances();
        solution_curr.print();
        solution_curr.printStatsDistances();

        auto p_locations = solution_curr.get_pLocations();
        auto p_locations_less_cap_used = getLessUsedCapacityLocations(p_locations, num_facilities_added, p_value);

        // Generate new locations for post-optimization and create a new instance
        auto new_locations = getLocationsPostOptimization(num_facilities_po, p_locations, p_locations_less_cap_used);
        // add p locations in new locations
        auto new_instance = createNewInstance(new_locations, solution_curr);

        // new locations without p locations to prioritize
        vector<uint_t> new_locations_no_p;
        for (auto loc : new_locations) {
            if (p_locations.find(loc) == p_locations.end()) {
                new_locations_no_p.push_back(loc);
            }
        }

        // Run PMP with the new instance and update solution
        PMP pmp = setupPMP(new_instance, Method_name, solution_curr.isSolutionFeasible(), solution_curr);
        pmp.set_PriorityListLocations(new_locations_no_p, "presence_based");
        pmp.run(Method_name);
        solution_curr = pmp.getSolution_cap();

        // Evaluate new solution
        if (!solution_curr.get_pLocations().empty()) {
            evaluateSolution(solution_curr, alpha);
        } else {
            handleNoSolutionFound(solution_curr, alpha);
        }

        iter++;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        cout << "Time elapsed: " << elapsed_seconds.count() << "s" << endl;
        this->timelimit -= elapsed_seconds.count();
        cout << "Time remaining: " << timelimit << "s" << endl;

        if (timelimit <= 0) {
            finish = true;
            cout << "\n[INFO] Time limit reached. Exiting post-optimization." << endl;
            break;
        }


    }
}

// Function to print iteration details
void PostOptimization::printIterationInfo(int iter, uint_t num_facilities_po, double alpha) const {
    cout << "\n\n[INFO] Post-Optimization Iteration: " << iter << endl;
    cout << "Num facilities Post Optimization: " << num_facilities_po << endl;
    cout << "Alpha value: " << alpha << endl;
}

// Helper function to get locations with the least capacity used
// Helper function to get locations with the least capacity used
vector<uint_t> PostOptimization::getLessUsedLocations(const unordered_set<uint_t>& p_locations, uint_t num_facilities_added) {
    unordered_map<uint_t, dist_t> loc_usages = this->solution_cap.getLocUsages(); // location -> usage
    vector<pair<dist_t, uint_t>> sorted_cap_p_locations;

    // Collect capacities of each p_location and sort them by capacity usage
    for (const auto& p_loc : p_locations) {
        auto cap = instance->getLocCapacity(p_loc);
        sorted_cap_p_locations.emplace_back(cap, p_loc);
    }

    // Sort by capacity usage (ascending)
    sort(sorted_cap_p_locations.begin(), sorted_cap_p_locations.end());

    // Extract the locations with the least usage
    vector<uint_t> less_used_locations;
    for (uint_t i = 0; i < num_facilities_added && i < sorted_cap_p_locations.size(); i++) {
        less_used_locations.push_back(sorted_cap_p_locations[i].second);
    }

    return less_used_locations;
}


// Function to get locations with less capacity usage
vector<uint_t> PostOptimization::getLessUsedCapacityLocations(
    const unordered_set<uint_t>& p_locations, uint_t num_facilities_added, uint_t p_value) {
    
    if (num_facilities_added >= p_value) {
        // If the number of facilities to be added is greater than or equal to the number of existing facilities,
        // return all locations in the unordered_set as a vector
        return vector<uint_t>(p_locations.begin(), p_locations.end());
    }
    
    // Otherwise, call the function that handles less-used capacity locations
    return getLessUsedLocations(p_locations, num_facilities_added);
}

vector<uint_t> PostOptimization::getLocationsPostOptimization(uint_t num_locations_po, unordered_set<uint_t> p_locations, vector<uint_t> p_locations_less_used_cap) {

    // If requested locations exceed total available locations, return all locations
    if (num_locations_po >= instance->getLocations().size()) {
        cout << "[INFO] Returning all locations" << endl;
        return instance->getLocations();
    }

    // Check if p_locations or p_locations_less_used_cap is empty
    if (p_locations.empty() || p_locations_less_used_cap.empty()) {
        cerr << "[ERROR] p_locations or p_locations_less_used_cap is empty" << endl;
        cout << "[WARN] Returning all locations" << endl;
        return instance->getLocations();
    }

    // Initialize variables
    vector<uint_t> final_locs;
    unordered_set<uint_t> final_locs_set;  // For fast lookup
    uint_t cont_loc = 0;

    // // print p loc and p loc less used cap
    cout << "P Locations: ";
    for (auto loc : p_locations) {
        cout << loc << " ";
    }
    cout << endl;
    cout << "P Locations Less Used Cap: ";
    for (auto loc : p_locations_less_used_cap) {
        cout << loc << " ";
    }
    cout << endl;

    // Ensure we haven't already fulfilled the required number of locations
    while (cont_loc < num_locations_po) {
        bool found_new_location = false;

        for (auto loc : p_locations_less_used_cap) {
            if (cont_loc >= num_locations_po) break;

            // Get the closest locations to the current `loc`
            auto k_locs = instance->get_kClosestLocations(loc, instance->getLocations().size() - 1);
            for (auto k_loc : k_locs) {
                // If the location is not in the final_locs_set, add it
                if (final_locs_set.find(k_loc) == final_locs_set.end()) {
                    final_locs.push_back(k_loc);
                    final_locs_set.insert(k_loc);
                    cont_loc++;
                    found_new_location = true;
                    break;  // Move to the next `loc` in p_locations_less_used_cap
                }
            }

            // Exit if we have reached the required number of locations
            if (cont_loc >= num_locations_po) break;
        }

        // Exit if no new unique locations were found in this iteration
        if (!found_new_location) {
            cerr << "[ERROR] Unable to find enough unique locations" << endl;
            break;
        }
    }

    // Count the number of locations in final_locs that are not in p_locations
    int cont = 0;
    for (auto loc : final_locs) {
        if (p_locations.find(loc) == p_locations.end()) {
            cont++;
        }
    }
    cout << "Number of different locations: " << cont << endl;

    return final_locs;
}


// Function to create a new instance with the post-optimized locations
Instance PostOptimization::createNewInstance(
    const vector<uint_t>& new_locations, Solution_cap& solution_curr) {

    auto new_instance = this->instance->getReducedSubproblem(new_locations, instance->getTypeService());
    new_instance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
    new_instance.set_ThresholdDist(solution_curr.getMaxDist());
    
    cout << "New instance PostOpt: " << endl;
    new_instance.print();
    
    return new_instance;
}

// Function to setup and return PMP with the new instance and solution
PMP PostOptimization::setupPMP(Instance& new_instance, const string& Method_name, bool is_feasible, Solution_cap& solution_curr) {
    PMP pmp(make_shared<Instance>(new_instance), "CPMP", isBinModel ? true : false);
    pmp.setGenerateReports(true);
    pmp.setAddBreakCallback(true);
    pmp.setTimeLimit(timelimit);

    if (is_feasible) {
        pmp.setMIPStartSolution(solution_curr);
        pmp.setBestBound(solution_curr.getBestBound());
    }
    
    return pmp;
}

// Function to evaluate the new solution
void PostOptimization::evaluateSolution(Solution_cap& solution_curr, double& alpha) {
    
    cout << "Post-Optimization Solution: " << endl;
    solution_curr.print();

    // Compare the new solution to the current one and update if it's improved
    if (solution_curr.get_objective() + 0.01 < this->solution_cap.get_objective()) {
        cout << "[INFO] Solution improved. Updating current solution." << endl;
        cout << "Comparing Solution Values: " << endl;
        cout << "Old Objective: " << this->solution_cap.get_objective() << endl;
        cout << "New Objective: " << solution_curr.get_objective() << endl;
        cout << "Comparing Best Bounds: " << endl;
        cout << "Old Best Bound: " << this->solution_cap.getBestBound() << endl;
        cout << "New Best Bound: " << solution_curr.getBestBound() << endl;
        this->solution_cap = solution_curr;  // Update the current solution
        alpha = 0.5;  // Reset alpha
    } else {
        cout << "[INFO] Solution not improved. Using last solution and increasing alpha." << endl;
        alpha += 0.5;  // Increase alpha if the solution hasn't improved
    }
}

// Function to handle the case when no solution is found
void PostOptimization::handleNoSolutionFound(Solution_cap& solution_curr, double& alpha) {
    cout << "[Warning] No feasible solution found. Using last solution and increasing alpha." << endl;
    solution_curr = this->solution_cap;  // Keep the last solution
    alpha += 0.5;  // Increase alpha
}



// old Run method
// void PostOptimization::run(string Method_name) {
//     cout << "[INFO] Running post-optimization" << endl;
//     cout << "Time limit: " << timelimit << endl;
//     cout << "Method: " << Method_name << endl;
//     cout << "Is Bin Model: " << isBinModel << endl;
//     cout << "Initial Solution: " << endl;
//     if (is_cap) {
//         this->solution_cap.print();
//     } else {
//         this->solution_std.print();
//     }
//     bool is_reduce_instance = false;

//     auto p = this->instance->get_p();
//     auto num_facilities = this->instance->getLocations().size();
//     int limit_neigh_size = ceil((num_facilities - p)/p); // limit of repeat solutions = num neighboors
//     cout << "Max size of the neigh: " << limit_neigh_size << endl;


//     // int cont_repeat_solution = 0;
//     int neigh_dist = 1;
//     int iter = 1;
//     while(timelimit > 0 && neigh_dist <= limit_neigh_size) { 

//         auto start = std::chrono::high_resolution_clock::now();

//         cout << " ------------------------------------ \n\n";
//         cout << "Iteration Post-Optimization: " << iter << endl;
//         cout << "Size of neighborhood: " << neigh_dist << endl;
        
//         Solution_cap solution_final = this->solution_cap;
//         this->solution_cap.statsDistances();
        
//         if (is_reduce_instance){
//             cout << "Reducing instance" << endl;
//             createSelectedLocations(neigh_dist);
//             // create new instance
//             auto new_instance = this->instance->getReducedSubproblem(selectedLocations, "null");
//             new_instance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
//             new_instance.set_ThresholdDist(instance->get_ThresholdDist());
//             this->solution_cap.statsDistances();
//             new_instance.set_ThresholdDist(this->solution_cap.getMaxDist());
//             new_instance.print();

//             PMP pmp(make_shared<Instance>(new_instance), "CPMP", true);
//             pmp.setGenerateReports(true);
//             pmp.setAddBreakCallback(true);
//             pmp.setTimeLimit(timelimit);

//             if (solution_final.isSolutionFeasible()) {
//                 pmp.setMIPStartSolution(solution_cap);
//                 pmp.set_pLocations_from_solution(solution_cap.get_pLocations());
//             } 

//             pmp.run(Method_name);
//             solution_final = pmp.getSolution_cap();

//         }else{
            
//             // PMP pmp(make_shared<Instance>(instance), "CPMP", true);
//             this->solution_cap.statsDistances();
//             instance->set_ThresholdDist(this->solution_cap.getMaxDist());
//             PMP pmp(instance, "CPMP", true);
//             pmp.setGenerateReports(true);
//             pmp.setAddBreakCallback(true);
//             pmp.setTimeLimit(timelimit);

//             if (solution_final.isSolutionFeasible()) {
//                 pmp.setMIPStartSolution(solution_cap);
//                 pmp.setBestBound(solution_cap.getBestBound());
//                 pmp.set_pLocations_from_solution(solution_cap.get_pLocations());
//                 pmp.set_MaxNeighbors_from_solution(neigh_dist);
//             }else{
//                 cout << "[WARN] Initial Solution is not feasible" << endl;
//             }


//             pmp.run(Method_name);
//             solution_final = pmp.getSolution_cap();
//         }

//         // check status solution_final
//         if (solution_final.get_pLocations().size() != 0) {
//             cout << "Post-Optimization Solution: " << endl;
//             solution_final.print();

//             if (solution_final.get_objective() < this->solution_cap.get_objective()) {
//                 this->solution_cap = solution_final;
//                 cout << "Comparing Best Bounds: " << endl;
//                 cout << "Old Best Bound: " << this->solution_cap.getBestBound() << endl;
//                 cout << "New Best Bound: " << solution_final.getBestBound() << endl;
//                 neigh_dist = 1;
//             }else{
//                 neigh_dist++;
//             }
//         }

//         if (neigh_dist > limit_neigh_size) {
//             cout << "[INFO] Repeat solution limit reached" << endl;
//             cout << "max number of neighboors is " << neigh_dist << endl;
//             break;
//         }
        
//         iter++;
//         auto end = std::chrono::high_resolution_clock::now();
//         std::chrono::duration<double> elapsed_seconds = end - start;
//         cout << "Time elapsed: " << elapsed_seconds.count() << "s" << endl;
//         cout << "Time remaining: " << timelimit << "s" << endl;
//         this->timelimit -= elapsed_seconds.count();
//     }
    
// }


void PostOptimization::run_partialOpt() {
    
    cout << "Running post-optimization (in parts)" << endl;
    

    vector<uint_t> visited_clusters;

    // while(timelimit > 60){


        // // get random 5 values in p locations from solution
        // vector<uint_t> p_locations = this->solution_cap.get_pLocations();
        // // vector<uint_t> p_locations_rand;
        // // random_shuffle(p_locations.begin(), p_locations.end());
        // // for (int i = 0; i < 5; i++) {
        // //     p_locations_rand.push_back(p_locations[i]);
        // // }

        // // chose a random p in p locations and add the 5 closest locations
        // auto p = p_locations[rand() % p_locations.size()];
        // auto k_locs = this->instance->get_kClosestLocations(p, 5); // with vecto p locations




        // // create a problem PMP with only the selected p locations not fixed
        // PMP pmp(make_shared<Instance>(*instance), "CPMP", true);

        // this->solution_cap.statsDistances();
        // instance->set_ThresholdDist(this->solution_cap.getMaxDist());
        // PMP pmp(instance, "CPMP", true);
        // pmp.setGenerateReports(true);

        // // set p locations fixed is the vector p locations without the 5 random values
        // vector<uint_t> p_locations_fixed;
        // for (auto p : p_locations) {
        //     if (find(p_locations_rand.begin(), p_locations_rand.end(), p) == p_locations_rand.end()) {
        //         p_locations_fixed.push_back(p);
        //     }
        // }
        // pmp.set_Fixed_pLocations_from_solution(p_locations_fixed);
        // if (solution_cap.getFeasibility()) {
        //     pmp.setMIPStartSolution(solution_cap);
        // }else{
        //     cout << "[WARN] Initial Solution is not feasible" << endl;
        // }


    // }



}




void PostOptimization::set_time_limit(double time) {
    this->timelimit = time;
}

Solution_cap PostOptimization::getSolution_cap() const {
    return this->solution_cap;
}

void PostOptimization::setBinModel(bool isBinModel) {
    this->isBinModel = isBinModel;
}