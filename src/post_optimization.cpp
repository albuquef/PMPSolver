#include "post_optimization.hpp"



// Constructor definitions
PostOptimization::PostOptimization(const std::shared_ptr<Instance>& instance, const Config& config, Solution_cap solution) 
    : config(config), is_cap(true), instance(instance), solution_cap(std::move(solution)) {
    std::cout << "PostOptimization constructor for Solution_cap" << std::endl;
    this->solution_cap.print();
}

PostOptimization::PostOptimization(const std::shared_ptr<Instance>& instance, const Config& config, Solution_std solution) 
    : config(config), is_cap(false), instance(instance), solution_std(std::move(solution)) {
    std::cout << "PostOptimization constructor for Solution_std" << std::endl;
    this->solution_std.print();
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
    cout << "[INFO] Running post-optimization" << endl;
    cout << "Time limit: " << timelimit << endl;
    bool is_reduce_instance = false;

    auto p = this->instance->get_p();
    auto num_facilities = this->instance->getLocations().size();
    int limit_neigh_size = ceil((num_facilities - p)/p); // limit of repeat solutions = num neighboors
    cout << "Max size of the neigh: " << limit_neigh_size << endl;


    // int cont_repeat_solution = 0;
    int neigh_dist = 1;
    int iter = 1;
    while(timelimit > 0 && neigh_dist <= limit_neigh_size) { 

        auto start = std::chrono::high_resolution_clock::now();

        cout << " ------------------------------------ \n\n";
        cout << "Iteration Post-Optimization: " << iter << endl;
        cout << "Size of neighborhood: " << neigh_dist << endl;
        
        Solution_cap solution_final = this->solution_cap;
        this->solution_cap.statsDistances();
        
        if (is_reduce_instance){
            cout << "Reducing instance" << endl;
            createSelectedLocations(neigh_dist);
            // create new instance
            auto new_instance = this->instance->getReducedSubproblem(selectedLocations, "null");
            new_instance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
            new_instance.set_ThresholdDist(instance->get_ThresholdDist());
            this->solution_cap.statsDistances();
            new_instance.set_ThresholdDist(this->solution_cap.getMaxDist());
            new_instance.print();

            PMP pmp(make_shared<Instance>(new_instance), "CPMP", true);
            pmp.setGenerateReports(true);
            pmp.setAddBreakCallback(true);
            pmp.setTimeLimit(timelimit);

            if (solution_final.isSolutionFeasible()) {
                pmp.setMIPStartSolution(solution_cap);
                pmp.set_pLocations_from_solution(solution_cap.get_pLocations());
            } 

            pmp.run(Method_name);
            solution_final = pmp.getSolution_cap();

        }else{
            
            // PMP pmp(make_shared<Instance>(instance), "CPMP", true);
            this->solution_cap.statsDistances();
            instance->set_ThresholdDist(this->solution_cap.getMaxDist());
            PMP pmp(instance, "CPMP", true);
            pmp.setGenerateReports(true);
            pmp.setTimeLimit(timelimit);

            if (solution_final.isSolutionFeasible()) {
                pmp.setMIPStartSolution(solution_cap);
                pmp.setBestBound(solution_cap.getBestBound());
                pmp.set_pLocations_from_solution(solution_cap.get_pLocations());
                pmp.set_MaxNeighbors_from_solution(neigh_dist);
            }else{
                cout << "[WARN] Initial Solution is not feasible" << endl;
            }


            pmp.run(Method_name);
            solution_final = pmp.getSolution_cap();
        }

        // check status solution_final
        if (solution_final.get_pLocations().size() != 0) {
            cout << "Post-Optimization Solution: " << endl;
            solution_final.print();

            if (solution_final.get_objective() < this->solution_cap.get_objective()) {
                this->solution_cap = solution_final;
                cout << "Comparing Best Bounds: " << endl;
                cout << "Old Best Bound: " << this->solution_cap.getBestBound() << endl;
                cout << "New Best Bound: " << solution_final.getBestBound() << endl;
                neigh_dist = 1;
            }else{
                neigh_dist++;
            }
        }

        if (neigh_dist > limit_neigh_size) {
            cout << "[INFO] Repeat solution limit reached" << endl;
            cout << "max number of neighboors is " << neigh_dist << endl;
            break;
        }
        
        iter++;
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        cout << "Time elapsed: " << elapsed_seconds.count() << "s" << endl;
        cout << "Time remaining: " << timelimit << "s" << endl;
        this->timelimit -= elapsed_seconds.count();
    }
    
}


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
