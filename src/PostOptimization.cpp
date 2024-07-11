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
        // selectedLocations.push_back(k_locs[num_k-1]);

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


    // // create new instance
    // auto new_instance = this->instance->getReducedSubproblem(selectedLocations, "null");
    // new_instance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
    // new_instance.print();

    // // create new solution
    // PMP pmp(make_shared<Instance>(new_instance), "CPMP", true);
    // pmp.setGenerateReports(true);
    // pmp.setTimeLimit(3600);

    // if (solution_cap.getFeasibility()) {
    //     pmp.setMIPStartSolution(solution_cap);
    // } 

    // // ADD INITIAL SOLUTION SOLUTION_CAP

    // // NOW ADD THE BEST BOUND FOUND IN CPLEX BEFORE


    // pmp.run("EXACT_CPMP_BIN");
    // auto solution = pmp.getSolution_cap();


    // cout << "Post-Optimization Solution: " << endl;
    // solution.print();

}


void PostOptimization::run() {
    cout << "Running post-optimization" << endl;

    if (timelimit > 60) {
        cout << "Time limit: " << timelimit << endl;
    }else{
        cout << "Finishing post-optimization" << endl;
        // return this->solution_cap;
    }


    int limit_repeat_soluttion = 3;
    int cont_repeat_solution = 0;

    int neigh_dist = 1;
    int iter = 1;
    double start_time = 0;
    while(timelimit > 60) {
        
        auto start = std::chrono::high_resolution_clock::now();
        cout << " ------------------------------------ \n\n";
        cout << "Iteration Post-Optimization: " << iter << endl;
        cout << "Size of neighborhood: " << neigh_dist << endl;
        createSelectedLocations(neigh_dist);
        // create new instance
        auto new_instance = this->instance->getReducedSubproblem(selectedLocations, "null");
        new_instance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
        new_instance.print();

        // create new solution
        PMP pmp(make_shared<Instance>(new_instance), "CPMP", true);
        pmp.setGenerateReports(true);
        pmp.setTimeLimit(timelimit);

        if (solution_cap.getFeasibility()) {
            pmp.setMIPStartSolution(solution_cap);
        } 

        // ADD INITIAL SOLUTION SOLUTION_CAP

        // NOW ADD THE BEST BOUND FOUND IN CPLEX BEFORE


        pmp.run("EXACT_CPMP_BIN");
        auto solution = pmp.getSolution_cap();


        cout << "Post-Optimization Solution: " << endl;
        solution.print();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        this->timelimit -= elapsed_seconds.count();


        if (solution.get_objective() < this->solution_cap.get_objective()) {
            this->solution_cap = solution;
            neigh_dist = 1;
            cont_repeat_solution = 0;
        }else{
            neigh_dist++;
            cont_repeat_solution++;
        }

        if (cont_repeat_solution >= limit_repeat_soluttion) {
            cout << "Repeat solution limit reached" << endl;
            cout << "number of repeat solutions: " << cont_repeat_solution << endl;
            break;
        }

        iter++;
    }
    
    cout << "Finishing post-optimization" << endl;
    this->solution_cap.print(); 

}


void PostOptimization::set_time_limit(double time) {
    this->timelimit = time;
}