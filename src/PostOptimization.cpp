#include "PostOptimization.hpp"



PostOptimization::PostOptimization(const shared_ptr<Instance>& instance, const Solution_cap solution) : instance(instance), is_cap(true), solution_cap(solution) {
    // this->env = IloEnv();
    // this->model = IloModel(env);
    // this->cplex = IloCplex(model);
    // this->pmpSolver = PMP(instance);
    cout << "PostOptimization constructor" << endl;
    this->solution_cap.print();


    cout << "-----------------------------------------------" << endl;
    cout << "Instance information" << endl;
    instance->print();


    createSelectedLocations();

}

PostOptimization::PostOptimization(const shared_ptr<Instance>& instance, const Solution_std solution) : instance(instance), is_cap(false), solution_std(solution) {
    cout << "PostOptimization constructor" << endl;
    this->solution_std.print();
}


PostOptimization::~PostOptimization() {
    // this->env.end();
}


void PostOptimization::createSelectedLocations() {
    cout << "Creating selected locations" << endl;
    
    vector<uint_t> selectedLocations;
    auto p_locations = this->solution_cap.get_pLocations();

    int num_loc = this->instance->getCustomers().size();
    cout << "Number of locations: " << num_loc << endl;
    int num_p = p_locations.size();
    int num_k = (int) max(1, num_loc / num_p);
    num_k=1;

    for (auto p : p_locations) {
        // get k locs more close to p
        auto k_locs = this->instance->get_kClosestLocations(p, num_k);
        selectedLocations.insert(selectedLocations.end(), k_locs.begin(), k_locs.end());
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


    // create new instance
    auto new_instance = this->instance->getReducedSubproblem(selectedLocations, "null");
    new_instance.set_isWeightedObjFunc(instance->get_isWeightedObjFunc());
    new_instance.print();

    // create new solution
    PMP pmp(make_shared<Instance>(new_instance), "CPMP", true);
    pmp.setGenerateReports(true);


    if (solution_cap.getFeasibility()) {
        pmp.setMIPStartSolution(solution_cap);
    } 

    // ADD INITIAL SOLUTION SOLUTION_CAP

    // NOW ADD THE BEST BOUND FOUND IN CPLEX BEFORE


    pmp.run("EXACT_CPMP_BIN");
    auto solution = pmp.getSolution_cap();


    cout << "Post-Optimization Solution: " << endl;
    solution.print();

}