#include "TB.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include <utility>

#include <chrono> // for time-related functions
using namespace std::chrono;


TB::TB(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
//    cout << "TB heuristic initialized\n";
//    instance->print();

    // type_eval_solution = "GAPrelax";
    // this->type_eval_solution = "Heuristic";


}

Solution_std TB::initRandomSolution() {
    // Sample p distinct locations
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }
    Solution_std sol(instance, p_locations);
    return sol;
}

Solution_cap TB::initRandomCapSolution() {
    // Sample p distinct locations
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }
    Solution_cap sol(instance, p_locations);
    return sol;
}

Solution_cap TB::initHighestCapSolution() {

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    vector<pair<uint_t, uint_t>> sorted_locations;
    for (auto loc:locations) {
        auto cap = instance->getLocCapacity(loc);
        sorted_locations.emplace_back(cap, loc);
    }
    sort(sorted_locations.begin(), sorted_locations.end());
    reverse(sorted_locations.begin(), sorted_locations.end());

    for (int i = 0; i < p; i++) {
        p_locations.insert(sorted_locations[i].second);
    }

    Solution_cap solut(instance, p_locations);

    return solut;
}


Solution_std TB::run(bool verbose, int MAX_ITE) {

    auto sol_best = initRandomSolution();
    sol_best = localSearch_std(sol_best, verbose, MAX_ITE);

    return sol_best;
}

Solution_cap TB::run_cap(bool verbose, int MAX_ITE) {

    auto sol_best = initHighestCapSolution();
    // auto sol_best = initRandomCapSolution();
    sol_best = localSearch_cap(sol_best, verbose, MAX_ITE);
    return sol_best;
}


Solution_std TB::localSearch_std(Solution_std sol_best, bool verbose, int MAX_ITE) {

    // cout << "TB local search uncapacitated started\n";

    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///


    auto locations = instance->getLocations();
    bool improved = true;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    int objectiveCpt = 0;
    int ite=0;

    auto start_time_total = high_resolution_clock::now();
    while (improved && ite < MAX_ITE) {

        ite++;
        improved = false;
        sol_cand = sol_best;
        auto start = tick();
        auto p_locations = sol_best.get_pLocations();

        auto start_time = high_resolution_clock::now();
        for (auto loc:locations) { // First improvement over locations
            // if (!p_locations.contains(loc)) {
            if (std::find(p_locations.begin(), p_locations.end(), loc) == p_locations.end()){
                for (auto p_loc:p_locations) { // Best improvement over p_locations
                    sol_tmp = sol_best;
                    sol_tmp.replaceLocation(p_loc, loc);
                    
                    if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { 
                        sol_cand = sol_tmp;
                        improved = true;
                        objectiveCpt = 0;
                    }
                    // else{
                    //     objectiveCpt++;

                    //     if(objectiveCpt == TOLERANCE_CPT){
                    //         break;
                    //     }
                    // }
                }
            }
             if (improved) {


                sol_best = sol_cand;

                auto current_time = high_resolution_clock::now();
                auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    // sol_best.print();
                    // sol_best.saveAssignment(output_filename, 0);
                    // sol_best.saveResults(output_filename, mode, elapsed_time,ite);
                    // break;
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    cout << "Num ite TB: " << ite << "\n";
                    return sol_best;
                }

                break;
            }else{
                auto current_time = high_resolution_clock::now();
                auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    // sol_best.print();
                    // sol_best.saveAssignment(output_filename, 0);
                    // sol_best.saveResults(output_filename, mode, elapsed_time,ite);
                    // break;
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    cout << "Num ite TB: " << ite << "\n";
                    return sol_best;
                }


            };
        }
        if (verbose) {
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            cout << "uncapacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
            // tock(start);
            cout << endl;
        }
        ite++;
        // cout << "ite TB_PMP: " << ite << "\n";
    }


    cout << "Final solution uncapacited TB: \n";
    auto current_time = high_resolution_clock::now();
    auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
    cout << "Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite TB: " << ite << "\n";
    sol_best.print();
    cout << "\n";


    return sol_best;
}




Solution_cap TB::localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE) {

    // cout << "TB local search capacitated started\n";
    
    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///

    verbose = VERBOSE;

    auto locations = instance->getLocations();
    bool improved = true;
    Solution_cap sol_cand;
    int ite = 0;

    auto start_time_total = high_resolution_clock::now();
    while (improved && ite < MAX_ITE) {
        
        improved = false;
        sol_cand = sol_best;
        auto start = tick();
        auto p_locations = sol_best.get_pLocations();

        vector<uint_t> p_locations_vec;
        p_locations_vec.reserve(p_locations.size());
        for (auto p_loc:p_locations) p_locations_vec.push_back(p_loc);

        auto start_time = high_resolution_clock::now();
        for (auto loc:locations) { // First improvement over locations
            // if (!p_locations.contains(loc)) {
            if (std::find(p_locations.begin(), p_locations.end(), loc) == p_locations.end()){    
                // #pragma omp parallel for
                for (auto p_loc:p_locations_vec) { // Best improvement over p_locations
                    Solution_cap sol_tmp = sol_best;
                    Solution_cap sol_tmp2 = sol_best;
                    if (sol_tmp.getTotalCapacity() - instance->getLocCapacity(p_loc) + instance->getLocCapacity(loc) >= instance->getTotalDemand()) {
                        // #pragma omp critical
                        sol_tmp.replaceLocation(p_loc, loc, "PMP");
                        if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { // LB1
                            // evaluate solution with GAP assignment
                            // sol_tmp2.replaceLocation(p_loc, loc, "GAPrelax");
                            sol_tmp2.replaceLocation(p_loc, loc, "heuristic");
                            // sol_tmp.GAP_eval();
                            if (sol_cand.get_objective() - sol_tmp2.get_objective() > TOLERANCE_OBJ) { // LB2
                                sol_cand = sol_tmp2;
                                improved = true;
                                // cout << "improved\n";
                            }
                        }

                        // replace location checking LB2
                        // #pragma omp critical
                        // sol_tmp.replaceLocation(p_loc, loc);
                        // if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) {
                        //     sol_cand = sol_tmp;
                        //     improved = true;
                        // }

                    }
                }
            }
            if (improved) {


                sol_best = sol_cand;

                auto current_time = high_resolution_clock::now();
                auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    // sol_best.print();
                    // sol_best.saveAssignment(output_filename, 0);
                    // sol_best.saveResults(output_filename, mode, elapsed_time,ite);
                    // break;
                    cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    cout << "Num ite TB: " << ite << "\n";
                    return sol_best;
                }

                break;
            }else{
                auto current_time = high_resolution_clock::now();
                auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    // sol_best.print();
                    // sol_best.saveAssignment(output_filename, 0);
                    // sol_best.saveResults(output_filename, mode, elapsed_time,ite);
                    // break;
                    cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    cout << "Num ite TB: " << ite << "\n";
                    return sol_best;
                }


            };
        }

        if (verbose) {
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            cout << "capacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
            // tock(start);
            cout << endl;
        }

        ite++;
        // cout << "ite TB_CPMP: " << ite << "\n";
    }


    cout << "Final solution capacited TB: \n";
    auto current_time = high_resolution_clock::now();
    auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
    cout << "Elapsed time: " << elapsed_time << " seconds\n";
    cout << "Num ite TB: " << ite << "\n";
    sol_best.print();
    cout << "\n";

    return sol_best;
}