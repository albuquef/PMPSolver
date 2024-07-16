#include "PostOptimization.hpp"



PostOptimization::PostOptimization(const shared_ptr<Instance>& instance, const Solution_cap solution) : instance(instance), is_cap(true), solution_cap(solution) {
    // this->env = IloEnv();
    // this->model = IloModel(env);
    // this->cplex = IloCplex(model);
    // this->pmpSolver = PMP(instance);
    cout << "PostOptimization constructor" << endl;
    this->solution_cap.print();
}

PostOptimization::PostOptimization(const shared_ptr<Instance>& instance, const Solution_std solution) : instance(instance), is_cap(false), solution_std(solution) {
    cout << "PostOptimization constructor" << endl;
    this->solution_std.print();
}


PostOptimization::~PostOptimization() {
    // this->env.end();
}


void PostOptimization::createSelectedLocations(int num_k) {
    cout << "Creating selected locations" << endl;
    
    // vector<uint_t> selectedLocations;
    auto p_locations = this->solution_cap.get_pLocations();

    // int num_loc = this->instance->getCustomers().size();
    // cout << "Number of locations: " << num_loc << endl;
    // int num_p = p_locations.size();
    // int num_k = (int) max(1, num_loc / num_p);
    // // num_k=1;

    // erase selected locations
    selectedLocations.clear();

    for (auto p : p_locations) {
        // get k locs more close to p
        auto k_locs = this->instance->get_kClosestLocations(p, num_k);

        // add k locations
        selectedLocations.insert(selectedLocations.end(), k_locs.begin(), k_locs.end());

        // add only the kth position
        // uint_t lastValue = k_locs.back();
        // uint_t lastValue = vet[vet.size() - 1];
        // selectedLocations.push_back(k_locs[num_k-1]);
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


void PostOptimization::run() {
    cout << "[INFO] Running post-optimization" << endl;
    cout << "Time limit: " << timelimit << endl;
 
    bool is_reduce_instance = false;


    int limit_repeat_soluttion = 12;
    int cont_repeat_solution = 0;

    int neigh_dist = 1;
    int iter = 1;

    auto start_time_total = std::chrono::high_resolution_clock::now();

    while(timelimit > 60) {
        
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
            // new_instance.set_ThresholdDist(instance->get_ThresholdDist());
            this->solution_cap.statsDistances();
            new_instance.set_ThresholdDist(this->solution_cap.getMaxDist());
            new_instance.print();

            PMP pmp(make_shared<Instance>(new_instance), "CPMP", true);
            pmp.setGenerateReports(true);
            pmp.setTimeLimit(timelimit);

            if (solution_cap.getFeasibility()) {
                pmp.setMIPStartSolution(solution_cap);
                pmp.set_pLocations_from_solution(solution_cap.get_pLocations());
            } 

            pmp.run("EXACT_CPMP_BIN");
            solution_final = pmp.getSolution_cap();

        }else{
            
            // PMP pmp(make_shared<Instance>(instance), "CPMP", true);
            this->solution_cap.statsDistances();
            instance->set_ThresholdDist(this->solution_cap.getMaxDist());
            PMP pmp(instance, "CPMP", true);
            pmp.setGenerateReports(true);
            pmp.setTimeLimit(timelimit);

            if (solution_cap.getFeasibility()) {
                pmp.setMIPStartSolution(solution_cap);
                pmp.setBestBound(solution_cap.getBestBound());
                pmp.set_pLocations_from_solution(solution_cap.get_pLocations());
                pmp.set_MaxNeighbors_from_solution(neigh_dist);
            }else{
                cout << "[WARN] Initial Solution is not feasible" << endl;
            }


            pmp.run("EXACT_CPMP_BIN");
            solution_final = pmp.getSolution_cap();
        }

        // check status solution_final
        if (solution_final.getFeasibility()) {
            cout << "Post-Optimization Solution: " << endl;
            solution_final.print();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            cout << "Time elapsed: " << elapsed_seconds.count() << "s" << endl;
            this->timelimit -= elapsed_seconds.count();

    
            if (solution_final.get_objective() < this->solution_cap.get_objective()) {
                this->solution_cap = solution_final;
                cout << "Comparing Best Bounds: " << endl;
                cout << "Old Best Bound: " << this->solution_cap.getBestBound() << endl;
                cout << "New Best Bound: " << solution_final.getBestBound() << endl;
                neigh_dist = 1;
                cont_repeat_solution = 0;
            }else{
                neigh_dist++;
                cont_repeat_solution++;
            }

            if (cont_repeat_solution > limit_repeat_soluttion) {
                cout << "Repeat solution limit reached" << endl;
                cout << "number of repeat solutions: " << cont_repeat_solution << endl;
                break;
            }
        }
        iter++;
    }
    

    auto end_time_total = std::chrono::high_resolution_clock::now();
    cout << "\n\n\n";
    cout << "Finishing post-optimization" << endl;
    this->solution_cap.print(); 
    cout << "Post Optimization Time elapsed: " << std::chrono::duration<double>(end_time_total - start_time_total).count() << "s" << endl;    

    cout << "Solution Stats" << endl;
    this->solution_cap.statsDistances();
    cout << "Max dist: " << this->solution_cap.getMaxDist() << endl;
    cout << "Min dist: " << this->solution_cap.getMinDist() << endl;
    cout << "avg of Avg dists: " << this->solution_cap.getAvgDist() << endl;
    cout << "avg Std dev dist: " << this->solution_cap.getStdDevDist() << endl;

    cout << "\n\n\n";


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
