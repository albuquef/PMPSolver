#include "TB.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <iomanip>
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


void writeReport_TB(const string& filename, dist_t objective, int num_ite, int num_solutions, dist_t time) {
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


Solution_std TB::localSearch_std(Solution_std sol_best, bool verbose, int MAX_ITE) {

    cout << "uncapacitated TB local search started\n";

    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///

    auto locations = instance->getLocations();
    bool improved = true;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    int objectiveCpt = 0;

    int ite=1;
    auto start_time_total = high_resolution_clock::now();
    while (improved && ite < MAX_ITE) {

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
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
            if (improved) {

                sol_best = sol_cand;

                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    cout << "Num ite uncapacited TB: " << ite << "\n";
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    return sol_best;
                }

                break;
            }else{

                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the uncapacitated TB  algorithm.\n";
                    cout << "Num ite uncapacited TB: " << ite << "\n";
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    return sol_best;
                }


            };
        }
        if (verbose) {
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            cout << "Num ite uncapacited TB: " << ite << "\n";
            cout << "uncapacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
            // tock(start);
            cout << endl;
        }
        ite++;
    }

    cout << "Num ite uncapacited TB: " << ite << "\n";
    return sol_best;
}




Solution_cap TB::localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE) {

    // cout << "TB local search capacitated started\n";
    
    string report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(sol_best.get_pLocations().size()) + ".csv";


    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///

    auto locations = instance->getLocations();
    bool improved = true;
    Solution_cap sol_cand;

    if (generate_reports)
        writeReport_TB(report_filename, sol_best.get_objective(), 0, solutions_map.getNumSolutions(), 0);


    int ite = 1;
    auto start_time_total = high_resolution_clock::now();
    while (improved && ite < MAX_ITE) {
        
        improved = false;
        sol_cand = sol_best;
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

                    if (sol_tmp.getTotalCapacity() - instance->getLocCapacity(p_loc) + instance->getLocCapacity(loc) >= instance->getTotalDemand()) {
                        // #pragma omp critical

                        unordered_set<uint_t> p_locations_test = sol_best.get_pLocations();
                        p_locations_test.erase(p_loc);
                        p_locations_test.insert(loc);
                        int index = solutions_map.pSetExists_index(p_locations_test);
                        if (index != -1){
                            
                            if(solutions_map.getObjectiveByIndex(index)  < sol_cand.get_objective()){
                                sol_cand = solutions_map.getSolution(index);
                                improved = true;
                            }

                        }else{
                            sol_tmp.replaceLocation(p_loc, loc, "PMP");
                            if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { // LB1
                                
                                Solution_cap sol_tmp2 = sol_best;
                                // evaluate solution with GAP assignment
                                sol_tmp2.replaceLocation(p_loc, loc, "GAPrelax");
                                // sol_tmp2.replaceLocation(p_loc, loc, "heuristic");
                                
                                solutions_map.addUniqueSolution(sol_tmp2);

                                if (verbose) {
                                    cout << "best solution candidate: \n";
                                    sol_tmp2.print();
                                    auto current_time = high_resolution_clock::now();
                                    auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
                                    cout << "Num ite capacited TB: " << ite << "\n";
                                    cout << "capacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
                                    cout << endl;
                                }


                                // sol_tmp.GAP_eval();
                                if (sol_cand.get_objective() - sol_tmp2.get_objective() > TOLERANCE_OBJ) { // LB2
                                    
     
                                    sol_cand = sol_tmp2;
                                    improved = true;
                          
                                    if (generate_reports)
                                        writeReport_TB(report_filename, sol_cand.get_objective(), ite, solutions_map.getNumSolutions(), duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
                                    


                                }
                            }
                        }

                    }
                }
            }
            if (improved) {


                sol_best = sol_cand;

                auto current_time = high_resolution_clock::now();
                auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the TB algorithm.\n";
                    cout << "Num ite capacited TB: " << ite << "\n";
                    cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    return sol_best;
                }

                break;
            }else{
                auto current_time = high_resolution_clock::now();
                auto elapsed_time = duration_cast<seconds>(current_time - start_time_total).count();
                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the TB algorithm.\n";
                    cout << "Num ite capacited TB: " << ite << "\n";
                    cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    cout << "Num ite TB: " << ite << "\n";
                    return sol_best;
                }


            };
        }

        if (verbose) {
            cout << "Solution Best: \n";
            sol_best.print();
            auto current_time = high_resolution_clock::now();
            auto elapsed_time = duration_cast<seconds>(current_time - start_time).count();
            cout << "Num ite capacited TB: " << ite << "\n";
            cout << "capacitated TB loop elapsed time: " << elapsed_time << " seconds\n";
            cout << "total time: " << duration_cast<seconds>(current_time - start_time_total).count() << " seconds\n";
            // tock(start);
            cout << endl;
        }

        
        ite++;
    }

    cout << "Num ite capacited TB: " << ite << "\n";
    return sol_best;
}

void TB::setSolutionMap(Solution_MAP sol_map) {
    this->solutions_map = sol_map;
}

void TB::setGenerateReports(bool generate_reports) {
    this->generate_reports = generate_reports;
}

void TB::setMethod(string Method) {
    this->typeMethod = Method;
}