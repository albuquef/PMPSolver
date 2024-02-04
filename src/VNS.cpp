#include "VNS.hpp"
#include <iomanip>
#include <chrono> // for time-related functions
using namespace std::chrono;

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

Solution_std VNS::rand_swap_Locations(Solution_std sol_current, int num_swaps, int seed){

    // Set the seed for the random number generator
    std::mt19937 rng(seed);

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
    std::vector<uint_t> indices_out = getDistinctIndices(out_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_out) out_swap_loc.push_back(out_locations_vec[i]);

    std::vector<uint_t> indices_p = getDistinctIndices(p_locations_vec.size(), num_swaps, seed);
    for (auto i:indices_p) p_swap_loc.push_back(p_locations_vec[i]);

    for (int i = 0; i < num_swaps; i++) {
        sol_current.replaceLocation(p_swap_loc[i], out_swap_loc[i]);
        // cout << "p_swap_loc: " << p_swap_loc[i] << " out_swap_loc: " << out_swap_loc[i] << "\n";
    }

    if (sol_current.get_pLocations().size() > p) return sol_current;

    return sol_current;
}
Solution_cap VNS::rand_swap_Locations_cap(Solution_cap sol_current, int num_swaps, int seed){

    // Set the seed for the random number generator
    std::mt19937 rng(seed);

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

    cout << "uncapacitated VNS heuristic started\n";

    TB tb(instance, engine());
    tb.setSolutionMap(solutions_map);
    auto sol_best = tb.initRandomSolution();
    cout << "Initial solution: \n";
    sol_best.print();

    // limit of neighborhoods
    int p = sol_best.get_pLocations().size();
    auto Kmax = int(sol_best.get_pLocations().size()/2);  // max number of locations to swap
    int k = 1; // initial neighborhood
    
    // limit of time and iterations
    // auto time_limit_seconds = 3600;
    auto time_limit_seconds = CLOCK_LIMIT;
    // int MAX_ITE_LOCAL = int(p/5);

    int ite = 1;
    auto start_time_total = high_resolution_clock::now();
    while(ite <= MAX_ITE){
    // while(ite <= 3){
        auto start_time = high_resolution_clock::now();
        auto sol_tmp = rand_swap_Locations(sol_best,k, ite);
        sol_tmp = tb.localSearch_std(sol_tmp,verbose,DEFAULT_MAX_ITE);


        if (sol_tmp.get_objective() < sol_best.get_objective()){
            sol_best = sol_tmp;
            k = 1;
        }else if(k <= Kmax){
            k++;
        }else if (k > Kmax){
            cout << "Limit of neighborhoods reached. Stopping the uncapacitated VNS algorithm.\n ";
            cout << " k = " << k << " > Kmax\n";
            return sol_best;
        }

        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
        if (elapsed_time >= time_limit_seconds) {
            cout << "Time limit reached. Stopping the uncapacitated VNS algorithm.\n";
            return sol_best;
        }

        if (verbose) {
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            cout << "uncapacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
        }

        ite++;
    }

    cout << "Final solution: \n";
    auto current_time = high_resolution_clock::now();
    auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
    cout << "Final Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite uncapacitated VNS: " << ite << "\n";
    sol_best.print();
    cout << "\n";
    return sol_best;
}


void writeReport(const string& filename, dist_t objective, int num_ite, int num_solutions, dist_t time) {
    // Open the file for writing in append mode
    ofstream outputFile(filename, ios::app);
    if (!outputFile.is_open()) {
        cerr << "Error opening the output file." << endl;
        return;
    }

    outputFile << fixed << setprecision(15) << objective << ";"; // obj value
    outputFile << num_ite << ";";
    outputFile << num_solutions << ";";
    outputFile << time << "\n";


    // Close the file
    outputFile.close();
}

bool VNS::isBetter_cap(Solution_cap sol_cand, Solution_cap sol_best){
    if (sol_cand.get_objective() < sol_best.get_objective()) return true;
    return false;
}
Solution_cap VNS::runVNS_cap(string output_filename, string& Method, bool verbose, int MAX_ITE) {

    cout << "capacitated VNS heuristic started\n";
    auto start_time_total = high_resolution_clock::now();
    
    // limit of time and iterations
    // auto time_limit_seconds = 3600;
    auto time_limit_seconds = CLOCK_LIMIT;
    // int MAX_ITE_LOCAL = int(p/5);
    cout << "Time Limit: " << time_limit_seconds << " seconds\n";

    TB tb(instance, engine());
    tb.setSolutionMap(solutions_map);
    tb.setMethod("TB_" + Method);
    auto start_time_v0 = high_resolution_clock::now();
    auto sol_best = tb.initHighestCapSolution();
    // auto sol_best = tb.initSmartRandomCapSolution();
    sol_best = tb.localSearch_cap(sol_best,true,10);
    tb.solutions_map.addUniqueSolution(sol_best);
    cout << "Initial solution: \n";
    sol_best.print();

    // exit(0);

    // limit of neighborhoods
    int p = sol_best.get_pLocations().size();
    auto Kmax = int(sol_best.get_pLocations().size()/2);  // max number of locations to swap
    // int k = 1; // initial neighborhood
    int k = int(sol_best.get_pLocations().size()/4);; // initial neighborhood

    string report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(p) + ".csv";

    if (generate_reports)
        writeReport(report_filename, sol_best.get_objective(), 0, tb.solutions_map.getNumSolutions(), duration_cast<seconds>(high_resolution_clock::now() - start_time_v0).count());


    int ite = 1;
    while (ite <= MAX_ITE) {
    // while (ite <= 10) {
        auto start_time = high_resolution_clock::now();
        cout << "vizinhanca: " << k << "\n";
        auto new_sol = rand_swap_Locations_cap(sol_best,k, ite);
        cout << "\nlocal search\n";
        new_sol = tb.localSearch_cap(new_sol,true,100);
        new_sol.print();
        

        if (generate_reports)
            writeReport(report_filename, new_sol.get_objective(), ite, tb.solutions_map.getNumSolutions(), duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());

        if (new_sol.get_objective() < sol_best.get_objective()) {
            // auto p_loc = new_sol.get_pLocations();
            // sol_best = Solution_cap(instance, p_loc);
        
            sol_best = new_sol;
            // k = 1;
            k = int(sol_best.get_pLocations().size()/4);; // initial neighborhood
        }
        if (k <= Kmax){
            k++;   
        }else if (k > Kmax){
            cout << "Limit of neighborhoods reached. Stopping the capacitated VNS algorithm.\n ";
            cout << " k = " << k << " > " << Kmax  << " = Kmax \n";
            return sol_best;
        }

        auto current_time = high_resolution_clock::now();
        auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();  
        if (elapsed_time >= time_limit_seconds) {
            cout << "Time limit reached. Stopping the capacitated VNS algorithm.\n";
            return sol_best;
        }

        if (verbose) {
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
            cout << "capacitated VNS loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
            // tock(start);
            cout << endl;
        }

        cout << "ite: " << ite << " k: " << k << " sol_best: " << sol_best.get_objective() << "\n";
        cout << "-------------------------------\n";
        cout << "size solutions_map: " << tb.solutions_map.getNumSolutions() << "\n";
        cout << "-------------------------------\n";

        ite++;
    }

    cout << "Final solution: \n";
    sol_best.print();
    cout << "\n";
    auto current_time = high_resolution_clock::now();
    auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
    cout << "Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite VNS: " << ite << "\n";

    return sol_best;

}

void VNS::setSolutionMap(Solution_MAP sol_map){
    solutions_map = sol_map;
}

void VNS::setMethod(string Method){
    this->typeMethod = Method;
}