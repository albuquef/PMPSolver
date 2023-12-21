#include "VNS.hpp"
#include <chrono> // for time-related functions
using namespace std::chrono;

VNS::VNS(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
}

vector<uint_t> getDistinctIndices(size_t vec_size, size_t k) {
    if (k > vec_size) {
        // Handle the case where k is greater than the vector size
        throw std::invalid_argument("k should be less than or equal to the vector size");
    }

    
    // Create a vector with the original indices
    std::vector<uint_t> original_indices(vec_size);
    std::iota(original_indices.begin(), original_indices.end(), 0);

    // Shuffle the vector of indices
    int seed_dist_ind = 42;
    std::default_random_engine rng(seed_dist_ind);
    std::shuffle(original_indices.begin(), original_indices.end(), rng);

    // Extract the first k elements (which are now shuffled)
    std::vector<uint_t> distinct_indices(original_indices.begin(), original_indices.begin() + k);

    return distinct_indices;
}



Solution_std VNS::rand_swap_Locations(Solution_std sol_current, int num_swaps){

    int p = sol_current.get_pLocations().size();
    auto locations = instance->getLocations();
    auto p_locations = sol_current.get_pLocations();

    vector<uint_t> p_locations_vec;
    p_locations_vec.reserve(p_locations.size());
    for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

    std::vector<size_t> excludeIndices = { static_cast<size_t>(-1) };
    // Copy locations to out_locations_vec excluding elements in p_locations
    std::vector<uint_t> out_locations_vec;
    std::copy_if(locations.begin(), locations.end(), std::back_inserter(out_locations_vec),
             [&p_locations](uint_t loc) { return p_locations.find(loc) == p_locations.end(); });

    // check if there are enough locations to swap
    if (out_locations_vec.size() < num_swaps || p_locations_vec.size() < num_swaps) return sol_current;

 
    vector<uint_t> out_swap_loc;
    vector<uint_t> p_swap_loc;
    out_swap_loc.reserve(num_swaps); 
    p_swap_loc.reserve(num_swaps); 

    // Generate 2 distinct indices for out_locations_vec
    std::vector<uint_t> indices_out = getDistinctIndices(out_locations_vec.size(), num_swaps);
    for (auto i:indices_out) out_swap_loc.push_back(out_locations_vec[i]);

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

    for (int i = 0; i < num_swaps; i++) {
        sol_current.replaceLocation(p_swap_loc[i], out_swap_loc[i]);
        // cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
    }

    if (sol_current.get_pLocations().size() > p) return sol_current;

    return sol_current;
}
Solution_cap VNS::rand_swap_Locations_cap(Solution_cap sol_current, int num_swaps){

    int p = sol_current.get_pLocations().size();
    auto locations = instance->getLocations();
    auto p_locations = sol_current.get_pLocations();

    vector<uint_t> p_locations_vec;
    p_locations_vec.reserve(p_locations.size());
    for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

    std::vector<size_t> excludeIndices = { static_cast<size_t>(-1) };
    // Copy locations to out_locations_vec excluding elements in p_locations
    std::vector<uint_t> out_locations_vec;
    std::copy_if(locations.begin(), locations.end(), std::back_inserter(out_locations_vec),
             [&p_locations](uint_t loc) { return p_locations.find(loc) == p_locations.end(); });

    if (out_locations_vec.size() < num_swaps || p_locations_vec.size() < num_swaps) return sol_current;

    vector<uint_t> out_swap_loc;
    vector<uint_t> p_swap_loc;
    out_swap_loc.reserve(num_swaps); 
    p_swap_loc.reserve(num_swaps); 

    // Generate 2 distinct indices for out_locations_vec
    std::vector<uint_t> indices_out = getDistinctIndices(out_locations_vec.size(), num_swaps);
    for (auto i:indices_out) out_swap_loc.push_back(out_locations_vec[i]);

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);


    dist_t add_Capacity = 0.0;
    dist_t lost_Capacity = 0.0;
    for (int i = 0; i < num_swaps; i++) {
        add_Capacity += instance->getLocCapacity(out_swap_loc[i]);
        lost_Capacity += instance->getLocCapacity(p_swap_loc[i]);
    }

    if (sol_current.getTotalCapacity() - lost_Capacity + add_Capacity >= instance->getTotalDemand()){
        for (int i = 0; i < num_swaps; i++) {
            sol_current.replaceLocation(p_swap_loc[i], out_swap_loc[i]);
            // cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
        }
    }else{
        cout << "Not enough capacity\n";
    }

    if (sol_current.get_pLocations().size() > p) return sol_current;

    return sol_current;
}

Solution_std VNS::runVNS_std(bool verbose, int MAX_ITE) {

    cout << "VNS heuristic started\n";

    TB tb(instance, engine());
    // auto locations = instance->getLocations();
    auto sol_best = tb.initRandomSolution();
    int ite = 0;

    cout << "Initial solution: \n";
    sol_best.print();
    cout << "\n";
    // cout << "p_locations: " << p_locations.size() << "\n";
    // int MAX_ITE_LS = 5;
    int MAX_ITE_LS = DEFAULT_MAX_ITE;
    int k = 1;
    auto Kmax = sol_best.get_pLocations().size();  // max number of locations to swap
    // auto Kmax = 10;  // max number of locations to swap


    while(ite <= MAX_ITE){
        auto sol_tmp = rand_swap_Locations(sol_best,k);
        sol_tmp = tb.localSearch_std(sol_tmp,verbose,DEFAULT_MAX_ITE);
        if (sol_tmp.get_objective() < sol_best.get_objective()){
            sol_best = sol_tmp;
            // cout << "ite: " << ite << " k: " << k << " sol_best: " << sol_best.get_objective() << "\n";
            // ite = 0;
            k = 1;
        }else if(k < Kmax){
            k++;
        }
        ite++;
    }

    cout << "Final solution: \n";
    sol_best.print();
    cout << "\n";

    // TB tb(instance, engine());
    // auto sol = tb.initRandomSolution();
    return sol_best;
}

bool VNS::isBetter_cap(Solution_cap sol_cand, Solution_cap sol_best){
    if (sol_cand.get_objective() < sol_best.get_objective()) return true;
    return false;
}



Solution_cap VNS::runVNS_cap(bool verbose, int MAX_ITE) {

    cout << "VNS heuristic capacitated started\n";
    
    TB tb(instance, engine());
    // auto locations = instance->getLocations();
    auto sol_best = tb.initHighestCapSolution();
    int ite = 0;

    cout << "Initial solution: \n";
    sol_best.print();
    cout << "\n";

    int p = sol_best.get_pLocations().size();


    auto Kmax = sol_best.get_pLocations().size();  // max number of locations to swap
    int k = 1;
    auto time_limit_seconds = 60;
    auto start_time = high_resolution_clock::now();
    while (ite <= MAX_ITE) {
        auto new_sol = rand_swap_Locations_cap(sol_best,k);
        new_sol = tb.localSearch_cap(new_sol,verbose,DEFAULT_MAX_ITE);
        // new_sol.GAP_eval();
        if (new_sol.get_objective() < sol_best.get_objective()) {
            sol_best = new_sol;
            // cout << "ite: " << ite << " k: " << k << " sol_best: " << sol_best.get_objective() << "\n";
            k = 1;
        }else if(k < Kmax){
            k++;
        }
        ite++;  

        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
        if (elapsed_time >= time_limit_seconds) {
            cout << "Time limit reached. Stopping the algorithm.\n";
            // sol_best.print();
            sol_best.saveAssignment("./solution/output", 0);
            break;
        }


    }

    cout << "Final solution: \n";
    sol_best.print();
    cout << "\n";

    return sol_best;

}

