#include "VNS.hpp"
#include <iomanip>
#include <chrono> // for time-related functions
using namespace std::chrono;


double get_wall_time_VNS(){
    struct timeval time;
    if(gettimeofday(&time,nullptr)){
        // HANDLE ERROR
        return 0;
    }else{
        return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_usec*0.000001); //microsegundos
    }
}
bool checkClock_VNS(double start, double limit) {
    auto end = get_wall_time_VNS();
    if (end - start >= limit) {
        cout << "\n[INFO] Time limit reached. Stopping the algorithm.\n";
        cout << "VNS elapsed time: " << end - start << " seconds\n";
        return true;
    }
    return false;
}


VNS::VNS(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
}
void VNS::setGenerateReports(bool generate_reports){
    this->generate_reports = generate_reports;
}   


vector<uint_t> getDistinctIndices(size_t vec_size, size_t k, int seed) {
    if (k > vec_size) {
        // Handle the case where k is greater than the vector size
        throw std::invalid_argument("k should be less than or equal to the vector size");
    }

    
    // Create a vector with the original indices
    std::vector<uint_t> original_indices(vec_size);
    std::iota(original_indices.begin(), original_indices.end(), 0);

    // Shuffle the vector of indices
    // int seed_dist_ind = 42;
    std::default_random_engine rng(seed);
    std::shuffle(original_indices.begin(), original_indices.end(), rng);

    // Extract the first k elements (which are now shuffled)
    std::vector<uint_t> distinct_indices(original_indices.begin(), original_indices.begin() + k);

    return distinct_indices;
}

// template<typename SolutionType>
// SolutionType VNS::Swap_Locations(SolutionType sol_current, unsigned int num_swaps, int seed, bool capac) {
    
//     // Set the seed for the random number generator
//     std::mt19937 rng(seed);

//     num_swaps = static_cast<unsigned int>(num_swaps);
//     auto locations = instance->getLocations();
//     auto p_locations = sol_current.get_pLocations();
//     auto p_locations_final = sol_current.get_pLocations();

//     vector<uint_t> p_locations_vec;
//     p_locations_vec.reserve(p_locations.size());
//     for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

//     std::vector<size_t> excludeIndices = { static_cast<size_t>(-1) };
//     // Copy locations to out_locations_vec excluding elements in p_locations
//     std::vector<uint_t> out_locations_vec;
//     std::copy_if(locations.begin(), locations.end(), std::back_inserter(out_locations_vec),
//              [&p_locations](uint_t loc) { return p_locations.find(loc) == p_locations.end(); });

//     if (out_locations_vec.size() < num_swaps || p_locations_vec.size() < num_swaps) {
//         cout << "[WARN] Not enough locations to swap\n";
//         return sol_current;
//     }

//     vector<uint_t> out_swap_loc; out_swap_loc.reserve(num_swaps); 
//     vector<uint_t> p_swap_loc; p_swap_loc.reserve(num_swaps); 

//     // Generate 2 distinct indices for out_locations_vec
//     std::vector<uint_t> indices_out = getDistinctIndices(out_locations_vec.size(), num_swaps, seed);
//     for (auto i:indices_out) out_swap_loc.push_back(out_locations_vec[i]);

//     cout << "\nout_swap_loc: ";
//     for (auto i:out_swap_loc) cout << i << " ";

//     std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
//     for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

//     cout << "\np_swap_loc: ";
//     for (auto i:p_swap_loc) cout << i << " ";


//     for (uint_t i = 0; i < num_swaps; i++) {
//         p_locations_final.erase(p_swap_loc[i]);
//         p_locations_final.insert(out_swap_loc[i]);
//     }

//     if (capac) {
//         dist_t add_Capacity = 0.0;
//         dist_t lost_Capacity = 0.0;
//         for (uint_t i = 0; i < num_swaps; i++) {
//             add_Capacity += instance->getLocCapacity(out_swap_loc[i]); 
//             lost_Capacity += instance->getLocCapacity(p_swap_loc[i]);
//         }
        
//         cout << "\nadd_Capacity: " << add_Capacity << " lost_Capacity: " << lost_Capacity << "\n";
//         // cout << "Total Capacity: " << sol_current.getTotalCapacity() << "\n";
//         cout << "Total Demand: " << instance->getTotalDemand() << "\n";

//         // if (sol_current.getTotalCapacity() - lost_Capacity + add_Capacity < instance->getTotalDemand()){
//         //     cout << "[WARN] Not enough capacity new swap solution\n";
//         //     return sol_current;
//         // }


//         // Solution_cap sol_cap = Solution_cap(instance, p_locations_final);
//         // sol_cap.setCoverMode(cover_mode);
//         // return sol_cap;

//     }


//     SolutionType sol_std = SolutionType(instance, p_locations_final);
//     sol_std.setCoverMode(cover_mode);
    
//     return sol_std;
// }

// template<typename SolutionType>
// SolutionType VNS::Swap_Locations_Cover(SolutionType sol_current, unsigned int num_swaps, int seed, bool capac){

//      // Set the seed for the random number generator
//     std::mt19937 rng(seed);

//     //change two locations inside p and outside p with the same subarea
//     auto locations = instance->getLocations();
//     auto p_locations = sol_current.get_pLocations();
//     auto p_locations_final = sol_current.get_pLocations();

//     vector<uint_t> p_locations_vec;
//     p_locations_vec.reserve(p_locations.size());
//     for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

//     std::vector<size_t> excludeIndices = { static_cast<size_t>(-1) };
//     // Copy locations to out_locations_vec excluding elements in p_locations
//     std::vector<uint_t> outside_p_locations_vec;
//     std::copy_if(locations.begin(), locations.end(), std::back_inserter(outside_p_locations_vec),
//              [&p_locations](uint_t loc) { return p_locations.find(loc) == p_locations.end(); });

//     if (outside_p_locations_vec.size() < num_swaps || p_locations_vec.size() < num_swaps){
//         cout << "[WARN] Not enough locations to swap\n";
//         return sol_current;
//     }

//     vector<uint_t> out_swap_loc; out_swap_loc.reserve(num_swaps);
//     vector<uint_t> p_swap_loc; p_swap_loc.reserve(num_swaps);

//     std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
//     for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

//     // generate indices with the same subarea outside p to swap
//     for (auto p_loc:p_swap_loc){
//         auto subarea = instance->getSubareaLocation(p_loc);
//         std::vector<uint_t> indices_out;
//         for (uint_t i = 0; i < outside_p_locations_vec.size(); i++){
//             if (instance->getSubareaLocation(outside_p_locations_vec[i]) == subarea) indices_out.push_back(i);
//         }
//         if (indices_out.size() > 0){
//             std::vector<uint_t> indices_out_dist = getDistinctIndices(indices_out.size(), 1, seed);
//             out_swap_loc.push_back(outside_p_locations_vec[indices_out[indices_out_dist[0]]]);
//         }else{
//             cout << "[WARN] No location with the same subarea to swap\n";
//             out_swap_loc.push_back(p_loc);
//         }
//     }

//     // print vectors
//     cout << "\np_swap_loc(cover): ";
//     for (auto i:p_swap_loc) cout << i << " ";
//     cout << "\nout_swap_loc(cover): ";
//     for (auto i:out_swap_loc) cout << i << " ";
//     cout << "\n";

//     if (out_swap_loc.size() < num_swaps) return sol_current;

//     for (uint_t i = 0; i < num_swaps; i++) {
//         p_locations_final.erase(p_swap_loc[i]);
//         p_locations_final.insert(out_swap_loc[i]);
//     }

//     if (capac){
//         dist_t add_Capacity = 0.0;
//         dist_t lost_Capacity = 0.0;
//         for (uint_t i = 0; i < num_swaps; i++) {
//             add_Capacity += instance->getLocCapacity(out_swap_loc[i]);
//             lost_Capacity += instance->getLocCapacity(p_swap_loc[i]);
//         }

//         // if (sol_current.getTotalCapacity() - lost_Capacity + add_Capacity < instance->getTotalDemand()){
//         //     cout << "[WARN] Not enough capacity new swap solution\n";
//         //     return sol_current;
//         // }

//         // Solution_cap sol_cap = Solution_cap(instance, p_locations_final);
//         // sol_cap.setCoverMode(cover_mode);
//         // return sol_cap;
//     }

//     SolutionType sol_std = SolutionType(instance, p_locations_final);
//     sol_std.setCoverMode(cover_mode);
//     return sol_std;
// }


Solution_std VNS::rand_swap_Locations(Solution_std sol_current, unsigned int num_swaps, int seed){

    // Set the seed for the random number generator
    std::mt19937 rng(seed);

    // num_swaps = static_cast<unsigned int>(num_swaps);
    auto locations = instance->getLocations();
    auto p_locations = sol_current.get_pLocations();
    auto p_locations_final = sol_current.get_pLocations();

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
    std::vector<uint_t> indices_out = getDistinctIndices(out_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_out) out_swap_loc.push_back(out_locations_vec[i]);

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);


    for (uint_t i = 0; i < num_swaps; i++) {
        p_locations_final.erase(p_swap_loc[i]);
        p_locations_final.insert(out_swap_loc[i]);
        // sol_current.replaceLocation(p_swap_loc[i], out_swap_loc[i]);
        // cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
    }
    sol_current = Solution_std(instance, p_locations_final);

    return sol_current;
}
Solution_std VNS::rand_swap_Locations_cover(Solution_std sol_current, unsigned int num_swaps, int seed){

        // Set the seed for the random number generator
    std::mt19937 rng(seed);

    //change two locations inside p and outside p with the same subarea
    auto locations = instance->getLocations();
    auto p_locations = sol_current.get_pLocations();
    auto p_locations_final = sol_current.get_pLocations();

    vector<uint_t> p_locations_vec;
    p_locations_vec.reserve(p_locations.size());
    for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

    std::vector<size_t> excludeIndices = { static_cast<size_t>(-1) };
    // Copy locations to out_locations_vec excluding elements in p_locations
    std::vector<uint_t> outside_p_locations_vec;
    std::copy_if(locations.begin(), locations.end(), std::back_inserter(outside_p_locations_vec),
             [&p_locations](uint_t loc) { return p_locations.find(loc) == p_locations.end(); });

    if (outside_p_locations_vec.size() < num_swaps || p_locations_vec.size() < num_swaps) return sol_current;

    vector<uint_t> out_swap_loc;
    vector<uint_t> p_swap_loc;
    out_swap_loc.reserve(num_swaps);
    p_swap_loc.reserve(num_swaps);

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

    // generate indices with the same subarea outside p to swap
    for (auto p_loc:p_swap_loc){
        auto subarea = instance->getSubareaLocation(p_loc);
        std::vector<uint_t> indices_out;
        for (uint_t i = 0; i < outside_p_locations_vec.size(); i++){
            if (instance->getSubareaLocation(outside_p_locations_vec[i]) == subarea) indices_out.push_back(i);
        }
        if (indices_out.size() > 0){
            std::vector<uint_t> indices_out_dist = getDistinctIndices(indices_out.size(), 1, seed);
            out_swap_loc.push_back(outside_p_locations_vec[indices_out[indices_out_dist[0]]]);
        }else{
            cout << "[WARN] No location with the same subarea to swap\n";
            out_swap_loc.push_back(p_loc);
        }
    }

    // print vectors
    cout << "\np_swap_loc(cover): ";
    for (auto i:p_swap_loc) cout << i << " ";
    cout << "\nout_swap_loc(cover): ";
    for (auto i:out_swap_loc) cout << i << " ";
    cout << "\n";
    if (out_swap_loc.size() < num_swaps) return sol_current;

    for (uint_t i = 0; i < num_swaps; i++) {
        p_locations_final.erase(p_swap_loc[i]);
        p_locations_final.insert(out_swap_loc[i]);
    }
    sol_current = Solution_std(instance, p_locations_final);


    return sol_current;
}
Solution_cap VNS::rand_swap_Locations_cap(Solution_cap sol_current, unsigned int num_swaps, int seed){

    // Set the seed for the random number generator
    std::mt19937 rng(seed);

    num_swaps = static_cast<unsigned int>(num_swaps);
    auto locations = instance->getLocations();
    auto p_locations = sol_current.get_pLocations();
    auto p_locations_final = sol_current.get_pLocations();

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
    std::vector<uint_t> indices_out = getDistinctIndices(out_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_out) out_swap_loc.push_back(out_locations_vec[i]);

    cout << "\nout_swap_loc: ";
    for (auto i:out_swap_loc) cout << i << " ";

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

    cout << "\np_swap_loc: ";
    for (auto i:p_swap_loc) cout << i << " ";

    dist_t add_Capacity = 0.0;
    dist_t lost_Capacity = 0.0;
    for (uint_t i = 0; i < num_swaps; i++) {
        add_Capacity += instance->getLocCapacity(out_swap_loc[i]);
        lost_Capacity += instance->getLocCapacity(p_swap_loc[i]);
    }

    if (sol_current.getTotalCapacity() - lost_Capacity + add_Capacity >= instance->getTotalDemand()){
        for (uint_t i = 0; i < num_swaps; i++) {
            p_locations_final.erase(p_swap_loc[i]);
            p_locations_final.insert(out_swap_loc[i]);
            // sol_current.replaceLocation(p_swap_loc[i], out_swap_loc[i]);
            // cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
        }
        sol_current = Solution_cap(instance, p_locations_final,"GAPrelax",cover_mode);
    }else{
        cout << "[WARN] Not enough capacity new swap solution\n";
    }

    return sol_current;
}
Solution_cap VNS::rand_swap_Locations_cap_cover(Solution_cap sol_current, unsigned int num_swaps, int seed){


    // Set the seed for the random number generator
    std::mt19937 rng(seed);

    //change two locations inside p and outside p with the same subarea
    auto locations = instance->getLocations();
    auto p_locations = sol_current.get_pLocations();
    auto p_locations_final = sol_current.get_pLocations();

    vector<uint_t> p_locations_vec;
    p_locations_vec.reserve(p_locations.size());
    for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

    std::vector<size_t> excludeIndices = { static_cast<size_t>(-1) };
    // Copy locations to out_locations_vec excluding elements in p_locations
    std::vector<uint_t> outside_p_locations_vec;
    std::copy_if(locations.begin(), locations.end(), std::back_inserter(outside_p_locations_vec),
             [&p_locations](uint_t loc) { return p_locations.find(loc) == p_locations.end(); });

    if (outside_p_locations_vec.size() < num_swaps || p_locations_vec.size() < num_swaps) return sol_current;

    vector<uint_t> out_swap_loc;
    vector<uint_t> p_swap_loc;
    out_swap_loc.reserve(num_swaps);
    p_swap_loc.reserve(num_swaps);

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

    // generate indices with the same subarea outside p to swap
    for (auto p_loc:p_swap_loc){
        auto subarea = instance->getSubareaLocation(p_loc);
        std::vector<uint_t> indices_out;
        for (uint_t i = 0; i < outside_p_locations_vec.size(); i++){
            if (instance->getSubareaLocation(outside_p_locations_vec[i]) == subarea) indices_out.push_back(i);
        }
        if (indices_out.size() > 0){
            std::vector<uint_t> indices_out_dist = getDistinctIndices(indices_out.size(), 1, seed);
            out_swap_loc.push_back(outside_p_locations_vec[indices_out[indices_out_dist[0]]]);
        }else{
            cout << "[WARN] No location with the same subarea to swap\n";
            out_swap_loc.push_back(p_loc);
        }
    }

    // print vectors
    cout << "\np_swap_loc(cover): ";
    for (auto i:p_swap_loc) cout << i << " ";
    cout << "\nout_swap_loc(cover): ";
    for (auto i:out_swap_loc) cout << i << " ";
    cout << "\n";
    if (out_swap_loc.size() < num_swaps) return sol_current;

    dist_t add_Capacity = 0.0;
    dist_t lost_Capacity = 0.0;
    for (uint_t i = 0; i < num_swaps; i++) {
        add_Capacity += instance->getLocCapacity(out_swap_loc[i]);
        lost_Capacity += instance->getLocCapacity(p_swap_loc[i]);
    }

    if (sol_current.getTotalCapacity() - lost_Capacity + add_Capacity >= instance->getTotalDemand()){
        for (uint_t i = 0; i < num_swaps; i++) {
            p_locations_final.erase(p_swap_loc[i]);
            p_locations_final.insert(out_swap_loc[i]);
        }
        sol_current = Solution_cap(instance, p_locations_final,"GAPrelax",cover_mode);
    }else{  
        cout << "[WARN] Not enough capacity in swap cover\n";
    }

    return sol_current;
}

template<typename SolutionType>
SolutionType runVNS(SolutionType sol_current, bool verbose, int MAX_ITE, int MAX_TIME, bool capac) {
    // auto start_time_total = high_resolution_clock::now();
    auto start_time_total = get_wall_time_VNS();
    int ite = 1;
    while (ite <= MAX_ITE) {
        // auto start_time = high_resolution_clock::now();
        auto start_time = get_wall_time_VNS();
        cout << "\n[INFO] Neighborhood(swap): " << ite << "\n";
        auto new_sol = Swap_Locations(sol_current, ite, ite, capac);
        auto elapsed_time = get_wall_time_VNS() - start_time;
        if (verbose) {
            cout << "\n[INFO] Best Solution found in Local Search (TB): \n";
            new_sol.print();
            cout << "Num ite VNS: " << ite << "\n";
            cout << "VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << endl;
        }

        if (new_sol.get_objective() < sol_current.get_objective()) {
            sol_current = new_sol;
        }

        if (checkClock_VNS(start_time_total, MAX_TIME)) {
            return sol_current;
        }

        ite++;
    }

    auto elapsed_time = get_wall_time_VNS() - start_time_total;
    cout << "\n[INFO] Final solution VNS: \n";
    sol_current.print();
    cout << "\n";
    cout << "Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite VNS: " << ite << "\n";
    cout << "-----------------------------------------------------------------\n";

    return sol_current;
}



Solution_std VNS::runVNS_std(bool verbose, int MAX_ITE) {

    cout << "\n[INFO] Uncapacitated VNS heuristic started\n";
    int p = instance->get_p();
    string report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(p) + ".csv";
    if (cover_mode) report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(p) + "_cover_"+ instance->getTypeSubarea() +".csv";

    // auto start_time_total = high_resolution_clock::now();
    auto start_time_total = get_wall_time_VNS();
    
    // limit of time and iterations
    // auto time_limit_seconds = 3600;
    auto time_limit_seconds = CLOCK_LIMIT;
    cout << "Time Limit: " << time_limit_seconds << " seconds\n";
    if (cover_mode) cout << "Cover Mode: ON " << "\n";

    TB tb(instance, engine());
    tb.setCoverMode(cover_mode);
    tb.setSolutionMap(solutions_map);
    // tb.setMethod("TB_" + Method);
    tb.setMethod("TB_");
    tb.setGenerateReports(true);
    
    Solution_std sol_best;
    if (cover_mode) sol_best = tb.initRandomSolution_Cover();
    else sol_best = tb.initRandomSolution();


    cout << "\n[INFO] Initial solution: \n";
    sol_best.print();
    

    // Solution_std sol_swap;
    //  if (cover_mode){
    //         sol_swap = rand_swap_Locations_cover(sol_best,2, 1);
    // }else{
    //     sol_swap = rand_swap_Locations(sol_best,2, 1);
    // }

    // cout << "\n[INFO] Swap initial solution: \n";   
    // sol_swap.print();

    // exit(0);


    bool local_search_initial_sol = true;
    if (local_search_initial_sol){
        cout << "\n[INFO] Local Search (initial solution)\n";
        sol_best = tb.localSearch_std(sol_best,true,DEFAULT_MAX_ITE);
        
        
        cout << "\n[INFO] Initial solution to VNS: \n";
        sol_best.print();
    }
    cout << "elapse time: " << get_wall_time_VNS() - start_time_total << " seconds\n\n";


    cout << "\n[INFO] Start loop VNS\n";
    // limit of neighborhoods
    auto Kmax = static_cast<unsigned int>(p/2); // max number of locations to swap
    unsigned int k = 2; // initial neighborhood
    cout << "K initial: " << k << "\n";
    cout << "Kmax: " << Kmax << "\n";

    int ite = 1;
    while (ite <= MAX_ITE) {
    // while (ite <= 10) {
        // auto start_time = high_resolution_clock::now();
        auto start_time = get_wall_time_VNS();
        cout << "\n[INFO] Neighborhood(swap): " << k << "\n";
        
        auto new_sol = sol_best;
        cout << "\n[INFO] Swap initial solution:\n";
        if (cover_mode){
            new_sol = rand_swap_Locations_cover(sol_best,k, ite);
        }else{
            new_sol = rand_swap_Locations(sol_best,k, ite);
        }
        
        new_sol.print();
        
        tb.setExternalTime(get_wall_time_VNS() - start_time_total);
        new_sol = tb.localSearch_std(new_sol,true,DEFAULT_MAX_ITE);

        auto elapsed_time = get_wall_time_VNS() - start_time;
        if (verbose) {
            cout << "\n[INFO] Best Solution found in Local Search (TB): \n";
            new_sol.print();
            cout << "Num ite uncapacited VNS: " << ite << "\n";
            cout << "uncapacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << endl;
        }


        auto elapsed_time_total = get_wall_time_VNS() - start_time_total;
        
        if (new_sol.get_objective() < sol_best.get_objective()) {
            
            sol_best = new_sol;
            
            // auto p_loc =  new_sol.get_pLocations();
            // sol_best = Solution_cap(instance, p_loc,"GAPrelax",cover_mode);
            // tb.solutions_map.addUniqueSolution(sol_best);

            if (verbose) {
                cout << "\n[INFO] Improved global solution in VNS: \n";
                sol_best.print();
                cout << "Num ite uncapacited VNS: " << ite << "\n";
                cout << "uncapacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
                cout << "elapsed time total: " << elapsed_time_total << " seconds\n";
                cout << endl;
            }

            k = 2;
            // k = int(sol_best.get_pLocations().size()/4); // initial neighborhood

        }
        if (k <= Kmax){
            k++;   
        }else if (k > Kmax){
            cout << "\n[INFO] Limit of neighborhoods reached. Stopping the uncapacitated VNS algorithm.\n ";
            cout << " k = " << k << " > " << Kmax  << " = Kmax \n";
            k = 2;
            // k = int(sol_best.get_pLocations().size()/4); // initial neighborhood
            // return sol_best;
        }

        if (verbose) {
            cout << "\n[INFO] Best global solution in VNS: \n";
            sol_best.print();
            cout << "Num ite uncapacited VNS: " << ite << "\n";
            cout << "uncapacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "elapsed time total: " << elapsed_time_total << " seconds\n";
            cout << "-----------------------------------------------------------------\n";
            cout << "size solutions_map: " << tb.solutions_map.getNumSolutions() << "\n";
            cout << "-----------------------------------------------------------------\n";
            cout << endl;
        }


        if (elapsed_time_total >= time_limit_seconds) {
            cout << "\n[INFO] Time limit reached. Stopping the uncapacitated VNS algorithm.\n";
            // auto p_loc =  sol_best.get_pLocations();
            // sol_best = Solution_cap(instance, p_loc,"GAPrelax",cover_mode);
            return sol_best;
        }

        ite++;
    }

    auto elapsed_time = get_wall_time_VNS() - start_time_total;
    cout << "\n[INFO] Final solution VNS: \n";
    sol_best.print();
    cout << "\n";
    cout << "Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite VNS: " << ite << "\n";
    cout << "-----------------------------------------------------------------\n";
    cout << "size solutions_map: " << tb.solutions_map.getNumSolutions() << "\n";
    cout << "-----------------------------------------------------------------\n";

    // auto p_loc =  sol_best.get_pLocations();
    // sol_best = Solution_cap(instance, p_loc,"GAPrelax",cover_mode);
    return sol_best;
}


void writeReport(const string& filename, dist_t objective, int num_ite, int num_solutions, double time) {
    // Open the file for writing in append mode
    ofstream outputFile(filename, ios::app);
    if (!outputFile.is_open()) {
        cerr << "Error opening the output file." << endl;
        return;
    }

    outputFile << fixed << setprecision(15) << objective << ";"; // obj value
    outputFile << num_ite << ";";
    outputFile << num_solutions << ";";
    outputFile << fixed << setprecision(15) << time << "\n";


    // Close the file
    outputFile.close();
}

bool VNS::isBetter_cap(Solution_cap sol_cand, Solution_cap sol_best){
    if (sol_cand.get_objective() < sol_best.get_objective()) return true;
    return false;
}
Solution_cap VNS::runVNS_cap(string& Method, bool verbose, int MAX_ITE) {

    cout << "\n[INFO] Capacitated VNS heuristic started\n";
    int p = instance->get_p();
    string report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(p) + ".csv";
    if (cover_mode) report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(p) + "_cover_"+ instance->getTypeSubarea() +".csv";

    // auto start_time_total = high_resolution_clock::now();
    auto start_time_total = get_wall_time_VNS();
    
    // limit of time and iterations
    // auto time_limit_seconds = 3600;
    auto time_limit_seconds = CLOCK_LIMIT;
    cout << "Time Limit: " << time_limit_seconds << " seconds\n";
    if (cover_mode) cout << "Cover Mode: ON " << "\n";

    TB tb(instance, engine());
    tb.setCoverMode(cover_mode);
    tb.setSolutionMap(solutions_map);
    tb.setMethod("TB_" + Method);
    tb.setGenerateReports(true);

    // Solution_cap sol_best;
    // sol_best = tb.fixedCapSolution();
    // if (cover_mode) sol_best = tb.initHighestCapSolution_Cover();
    // else sol_best = tb.initHighestCapSolution();
        
    auto sol_best = tb.initCPLEXCapSolution(600,"CPMP"); if(sol_best.isSolutionFeasible()) tb.solutions_map.addUniqueSolution(sol_best);
    // auto sol_best = tb.initSmartRandomCapSolution();
    // exit(0);

    cout << "\n[INFO] Initial solution: \n";
    sol_best.print();
    
    if (generate_reports) 
        writeReport(report_filename, sol_best.get_objective(), 0, tb.solutions_map.getNumSolutions(), get_wall_time_VNS() - start_time_total);  
    
    bool local_search_initial_sol = true;
    if (local_search_initial_sol){
        cout << "\n[INFO] Local Search (initial solution)\n";
        sol_best = tb.localSearch_cap(sol_best,true,DEFAULT_MAX_ITE);
        
        // if(sol_best.isSolutionFeasible()) 
        //     tb.solutions_map.addUniqueSolution(sol_best);
        
        cout << "\n[INFO] Initial solution - VNS: \n";
        sol_best.print();
        if (generate_reports) writeReport(report_filename, sol_best.get_objective(), 0, tb.solutions_map.getNumSolutions(), get_wall_time_VNS() - start_time_total);  
    }
    cout << "Elapse time: " << get_wall_time_VNS() - start_time_total << " seconds\n\n";

    cout << "\n[INFO] Start loop VNS\n";
    // limit of neighborhoods
    auto Kmax = static_cast<unsigned int>(p/2); // max number of locations to swap
    unsigned int k = 2; // initial neighborhood
    cout << "K initial: " << k << "\n";
    cout << "Kmax: " << Kmax << "\n";
   
    int ite = 1;
    while (ite <= MAX_ITE) {
    // while (ite <= 10) {
        // auto start_time = high_resolution_clock::now();
        auto start_time = get_wall_time_VNS();
        cout << "\n[INFO] Neighborhood(swap): " << k << "\n";
        
        auto new_sol = sol_best;
        cout << "\n[INFO] Swap initial solution:\n";
        if (cover_mode){
            new_sol = rand_swap_Locations_cap_cover(sol_best,k, ite);
        }else{
            new_sol = rand_swap_Locations_cap(sol_best,k, ite);
        }

        if(new_sol.isSolutionFeasible()) 
            tb.solutions_map.addUniqueSolution(sol_best);
        new_sol.print();
        
        tb.setExternalTime(get_wall_time_VNS() - start_time_total);
        new_sol = tb.localSearch_cap(new_sol,true,DEFAULT_MAX_ITE);

        auto elapsed_time = get_wall_time_VNS() - start_time;
        if (verbose) {
            cout << "\n[INFO] Best Solution found in Local Search (TB): \n";
            new_sol.print();
            cout << "Num ite capacited VNS: " << ite << "\n";
            cout << "capacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << endl;
        }


        auto elapsed_time_total = get_wall_time_VNS() - start_time_total;
        if (generate_reports)
            writeReport(report_filename, new_sol.get_objective(), ite, tb.solutions_map.getNumSolutions(), elapsed_time_total);

        
        if (new_sol.get_objective() < sol_best.get_objective()) {
            
            sol_best = new_sol;
            
            // auto p_loc =  new_sol.get_pLocations();
            // sol_best = Solution_cap(instance, p_loc,"GAPrelax",cover_mode);
            // tb.solutions_map.addUniqueSolution(sol_best);

            if (verbose) {
                cout << "\n[INFO] Improved global solution in VNS: \n";
                sol_best.print();
                cout << "Num ite capacited VNS: " << ite << "\n";
                cout << "capacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
                cout << "elapsed time total: " << elapsed_time_total << " seconds\n";
                if (sol_best.isSolutionFeasible()){
                    cout << "Improved solution feasible \n";
                }else{
                    cout << "Improved solution not feasible\n";
                }
                cout << endl;
            }

            k = 2;
            // k = int(sol_best.get_pLocations().size()/4); // initial neighborhood

        }
        if (k <= Kmax){
            k++;   
        }else if (k > Kmax){
            cout << "\n[INFO] Limit of neighborhoods reached. Stopping the capacitated VNS algorithm.\n ";
            cout << " k = " << k << " > " << Kmax  << " = Kmax \n";
            k = 2;
            // k = int(sol_best.get_pLocations().size()/4); // initial neighborhood
            // return sol_best;
        }

        if (verbose) {
            cout << "\n[INFO] Best global solution in VNS: \n";
            sol_best.print();
            cout << "Num ite capacited VNS: " << ite << "\n";
            cout << "capacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "elapsed time total: " << elapsed_time_total << " seconds\n";
            cout << "-----------------------------------------------------------------\n";
            cout << "size solutions_map: " << tb.solutions_map.getNumSolutions() << "\n";
            cout << "-----------------------------------------------------------------\n";
            cout << endl;
        }


        if (elapsed_time_total >= time_limit_seconds) {
            cout << "\n[INFO] Time limit reached. Stopping the capacitated VNS algorithm.\n";
            // auto p_loc =  sol_best.get_pLocations();
            // sol_best = Solution_cap(instance, p_loc,"GAPrelax",cover_mode);
            return sol_best;
        }

        ite++;
    }

    auto elapsed_time = get_wall_time_VNS() - start_time_total;
    cout << "\n[INFO] Final solution VNS: \n";
    sol_best.print();
    cout << "\n";
    cout << "Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite VNS: " << ite << "\n";
    cout << "-----------------------------------------------------------------\n";
    cout << "size solutions_map: " << tb.solutions_map.getNumSolutions() << "\n";
    cout << "-----------------------------------------------------------------\n";

    // auto p_loc =  sol_best.get_pLocations();
    // sol_best = Solution_cap(instance, p_loc,"GAPrelax",cover_mode);
    return sol_best;

}

void VNS::setSolutionMap(Solution_MAP sol_map){
    solutions_map = sol_map;
}

void VNS::setMethod(string Method){
    this->typeMethod = Method;
}

void VNS::setCoverMode(bool cover_mode){
    this->cover_mode = cover_mode;
}