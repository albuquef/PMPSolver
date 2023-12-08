#include "VNS.hpp"


VNS::VNS(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
    cout << "VNS heuristic initialized\n";
}

// vector<uint_t> getDistinctIndices(size_t maxIndex, size_t numIndices) {
//     std::random_device rd;
//     std::mt19937 gen(rd());
    
//     std::vector<uint_t> result;

//     while (result.size() < numIndices) {
//         std::uniform_int_distribution<uint_t> distribution(0, maxIndex - 1);
//         size_t index = distribution(gen);

//         if (std::find(result.begin(), result.end(), index) == result.end()) {
//             result.push_back(index);
//         }
//     }

//     return result;
// }

vector<uint_t> getDistinctIndices(size_t vec_size, size_t k) {
    if (k > vec_size) {
        // Handle the case where k is greater than the vector size
        throw std::invalid_argument("k should be less than or equal to the vector size");
    }

    // Create a vector with the original indices
    std::vector<uint_t> original_indices(vec_size);
    std::iota(original_indices.begin(), original_indices.end(), 0);

    // Shuffle the vector of indices
    std::random_device rd;
    std::default_random_engine rng(rd());
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
        cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
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
            cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
        }
    }else{
        cout << "Not enough capacity\n";
    }

    if (sol_current.get_pLocations().size() > p) return sol_current;

    return sol_current;
}

Solution_std VNS::runVNS_std(bool verbose, int MAX_ITE) {

    TB tb(instance, engine());
    auto locations = instance->getLocations();
    auto sol_best = tb.initRandomSolution();
    auto best_value = sol_best.get_objective();
    int ite = 0;
    auto p_locations = sol_best.get_pLocations();

    cout <<"\n";
    sol_best.print();
    cout <<"\n";
    auto sol_tmp = rand_swap_Locations(sol_best,2);
    cout <<"\n";
    sol_tmp.print();
    // cout <<"\n";
    // auto sol_tmp2 = rand_swap_Locations(sol_tmp,3);
    // sol_tmp2.print();
    cout <<"\n";

    cout << "p_locations: " << p_locations.size() << "\n";

    // while (ite <= MAX_ITE) {
    //     int ite++;
    //     while (k <= 3) {
    //         auto new_sol = sol.shake(k);
    //         new_sol = new_sol.localSearch_std();
    //         if (new_sol.getCost() < best_cost) {
    //             best_sol = new_sol;
    //             best_cost = new_sol.getCost();
    //             k = 1;
    //         } else {
    //             k++;
    //         }
    //     }
    //     sol = best_sol;
    //     ite++;
    // }
    // return best_sol;

    // TB tb(instance, engine());
    // auto sol = tb.initRandomSolution();
    return sol_tmp;
}

Solution_cap VNS::runVNS_cap(bool verbose, int MAX_ITE) {

    TB tb(instance, engine());
    auto locations = instance->getLocations();
    auto sol_best = tb.initHighestCapSolution();
    auto best_value = sol_best.get_objective();
    int ite = 0;
    auto p_locations = sol_best.get_pLocations();

    cout <<"\n";
    sol_best.print();
    cout <<"\n";
    auto sol_tmp = rand_swap_Locations_cap(sol_best,2);
    cout <<"\n";
    sol_tmp.print();
    cout <<"\n";
    auto sol_tmp2 = rand_swap_Locations_cap(sol_tmp,3);
    sol_tmp2.print();
    cout <<"\n";



    return sol_tmp2;

}

