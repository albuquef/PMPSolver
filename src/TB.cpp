#include "TB.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <iomanip>
#include <utility>

#include <chrono> // for time-related functions
using namespace std::chrono;

double get_wall_time_TB(){
    struct timeval time;
    if(gettimeofday(&time,nullptr)){
        // HANDLE ERROR
        return 0;
    }else{
        return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_usec*0.000001); //microsegundos
    }
}
void tock_TB(double start) {
    auto end = get_wall_time_TB();
    cout << "Elapsed time: " << end - start << " seconds\n";
}
template <typename SolutionType>
void printSolution_TB(SolutionType sol,double wallTime) {
    // cout << "Solution: \n";
    sol.print();
    cout << "Elapsed time: " << wallTime << " seconds\n";
    // cout << "Num ite: " << numIter << "\n";
}
void writeReport_TB(const string& filename, dist_t objective, int num_ite, int num_solutions, double time) {
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

    sol.setCoverMode(cover_mode);

    return sol;
}

Solution_std TB::initRandomSolution_Cover() {
    // Sample p distinct locations

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();
    auto unique_subareas = instance->getSubareasSet();
    // auto num_subareas = unique_subareas.size();
    uint_t cont_p = 0;

    vector<pair<uint_t, uint_t>> hight_loc_each_cover;
    for (auto subarea:unique_subareas) {
        auto locs = instance->getLocationsSubarea(subarea);
        vector<pair<uint_t, uint_t>> subareas_locations;
        for (auto loc:locs) {
            subareas_locations.emplace_back(0, loc);
        }
        if (subareas_locations.size() > 0){
            uniform_int_distribution<uint_t> distribution (0, subareas_locations.size() - 1);
            auto loc_id = distribution(engine);
            auto loc = subareas_locations[loc_id].second;
            p_locations.insert(loc);
            cont_p++;
        }else{
            cout << "subarea without locations\n";
            cout << "subarea: " << subarea << "\n";
        }

    }

    uniform_int_distribution<uint_t> distribution (0, locations.size() - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = locations[loc_id];
        p_locations.insert(loc);
    }

    Solution_std sol(instance, p_locations);
    sol.setCoverMode(cover_mode);
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
    Solution_cap sol(instance, p_locations,"GAPrelax",cover_mode);
    // sol.setCoverMode(cover_mode);
    return sol;
}

Solution_cap TB::initHighestCapSolution() {

    cout << "Initial Solution Highest Cap \n";

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

    for (uint_t i = 0; i < p; i++) {
        p_locations.insert(sorted_locations[i].second);
    }

    Solution_cap solut(instance, p_locations);
    // solut.setCoverMode(cover_mode);
    return solut;
}

Solution_cap TB::initSmartRandomCapSolution(){

    cout << "Initial Solution Smart Random \n";

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();
    auto filter_locations = instance->getLocations();  

    vector<pair<uint_t, uint_t>> sorted_locations;
    for (auto loc:locations) {
        auto cap = instance->getLocCapacity(loc);
        sorted_locations.emplace_back(cap, loc);
        if (cap <= 1){
            filter_locations.erase(std::remove(filter_locations.begin(), filter_locations.end(), loc), filter_locations.end());
        }
    }
    sort(sorted_locations.begin(), sorted_locations.end());
    reverse(sorted_locations.begin(), sorted_locations.end());

    // p_locations = {1,2,3,4};
    // Solution_cap solut(instance, p_locations, "GAPrelax");

    // uniform_int_distribution<uint_t> distribution (0, filter_locations.size() - 1);
    uniform_int_distribution<uint_t> distribution (0, min(90, (int)filter_locations.size()) - 1);
    while (p_locations.size() < p) {
        auto loc_id = distribution(engine);
        auto loc = filter_locations[loc_id];
        p_locations.insert(loc);
    }

    Solution_cap solut(instance, p_locations, "GAPrelax",cover_mode);
    bool feasible = solut.getFeasibility();
    while(!feasible){
        
            vector<pair<uint_t, uint_t>> sorted_p_locations;
            for (auto p_loc:solut.get_pLocations()) {
                auto cap = instance->getLocCapacity(p_loc);
                sorted_p_locations.emplace_back(cap, p_loc);
            }
            sort(sorted_p_locations.begin(), sorted_p_locations.end());
            if (sorted_p_locations[0].second != sorted_locations[0].second){
                solut.replaceLocation(sorted_p_locations[0].second, sorted_locations[0].second, "GAPrelax");
                if (!sorted_locations.empty()) {sorted_locations.erase(sorted_locations.begin());}
                feasible = solut.getFeasibility();
            }

            if (solut.getFeasibility() == true){
                cout << "feasible solution found\n";
            }else{
                cout << "infeasible solution found\n";
            }
    }

    solut.setCoverMode(cover_mode);
    return solut;


}

Solution_cap TB::initHighestCapSolution_Cover() {

    cout << "Initial Solution Highest Cap Coverages\n";
    if (instance->isCoverMode() == false){
        cout << "Instance is not in cover mode\n";
        initHighestCapSolution();
    }

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    auto filtered_locations = locations; // Filter n locations according to voting weights
    // cout << "[TB cpp] Filtered " << filtered_locations.size() << " locations: ";
    // for (auto fl:filtered_locations) cout << fl << " ";
    // cout << endl << endl;


    auto unique_subareas = instance->getSubareasSet();
    auto num_subareas = unique_subareas.size();
    uint_t cont_p = 0;

    vector<pair<uint_t, uint_t>> hight_loc_each_cover;
    for (auto subarea:unique_subareas) {
        auto locs = instance->getLocationsSubarea(subarea);
        // cout << "subarea: " << subarea << " size: " << locs.size() << "\n";
        // print vector locs
        // for (auto loc:locs) cout << loc << " ";
        // cout << "\n";
        vector<pair<uint_t, uint_t>> sorted_locations;
        for (auto loc:locs) {
            if (find(filtered_locations.begin(), filtered_locations.end(), loc) != filtered_locations.end()) {
                auto cap = instance->getLocCapacity(loc);
                sorted_locations.emplace_back(cap, loc);
            }
        }
        sort(sorted_locations.begin(), sorted_locations.end());
        reverse(sorted_locations.begin(), sorted_locations.end());
        hight_loc_each_cover.emplace_back(sorted_locations[0].first, sorted_locations[0].second);
        // cout << "add loc: " << sorted_locations[0].second  << "\n";
    }

    for (uint_t i = 0; i < num_subareas; i++) {
        p_locations.insert(hight_loc_each_cover[i].second);
        cont_p++;
    }

    while(p_locations.size() < p){
        vector<pair<uint_t, uint_t>> sorted_locations_diff;
        for (auto loc : locations) {
            // Check if loc exists in p_locations
            if (p_locations.find(loc) == p_locations.end() && find(filtered_locations.begin(), filtered_locations.end(), loc) != filtered_locations.end()) {
                auto cap = instance->getLocCapacity(loc);
                sorted_locations_diff.emplace_back(cap, loc);
            }
        }
        sort(sorted_locations_diff.begin(), sorted_locations_diff.end());
        reverse(sorted_locations_diff.begin(), sorted_locations_diff.end());
        for (uint_t i = 0; i < (p-num_subareas); i++) {
            p_locations.insert(sorted_locations_diff[i].second);
            cont_p++;
        }
    }
  
    // cout << "p_locations rest: ";
    // for (auto loc:p_locations) cout << loc << " ";
    // cout << "\n";

    Solution_cap solut(instance, p_locations);
    solut.setCoverMode(cover_mode);
    // cout << "aqui" << endl;
    // solut.print();
    return solut;
}

Solution_cap TB::fixedCapSolution(){

    cout << "Initial Solution Fixed Cap\n";

    unordered_set<uint_t> p_locations;

    // p_locations = {469,1050, 602, 663, 23, 214, 897, 872, 927, 1038, 842, 213, 156, 637, 359, 951, 952, 878, 414, 1302, 1197, 283, 801, 246, 949, 1245};
    p_locations = {469,787,602,663,23,214,897,872,927,1038,842,213,156,637,359,951,952,878,414,1302,1197,283,801,246,949,1245};
    // p_locations = {1206, 482, 1074, 1342, 1345, 516, 243, 432, 511, 474, 1325, 1214, 772, 884, 842 ,689 ,1318, 1011, 665, 951, 302, 637, 1414, 1269, 414, 525};

    Solution_cap solut(instance, p_locations);
    solut.setCoverMode(cover_mode);
    return solut;
}


Solution_cap TB::initCPLEXCapSolution(double time_limit, const char* typeProb) {
    
    CLOCK_LIMIT_CPLEX = time_limit;
    // PMP pmp(instance, "CPMP");
    PMP pmp(instance, typeProb);
    pmp.setGenerateReports(true);
    pmp.setCoverModel(cover_mode, instance->getTypeSubarea());
    pmp.run();
    // pmp.saveVars("cplex_inirmtial_sol_vars", "CPLEX");
    auto sol = pmp.getSolution_cap();
    // sol.saveAssignment("cplex_initial_sol", "CPLEX");
    // auto sol = Solution_cap(instance, pmp.getSolution_cap().get_pLocations());
    sol.setCoverMode(cover_mode);
    // sol.saveAssignment("cplex_initial_sol", "CPLEX");
    return sol;
}


Solution_std TB::run(bool verbose, int MAX_ITE) {

    cout << "uncapacitated TB started\n";

    Solution_std sol_best;
    if (!cover_mode) sol_best = initRandomSolution();
    if (cover_mode) sol_best = initRandomSolution_Cover();
    sol_best.setCoverMode(cover_mode);
    sol_best.print();

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

    cout << "uncapacitated TB local search started\n";

    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///

    auto locations = instance->getLocations();
    bool improved = true;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    // int objectiveCpt = 0;

    int ite=1;
    auto start_time_total = high_resolution_clock::now();
    while (improved && ite < MAX_ITE) {

        improved = false;
        sol_cand = sol_best;
        // auto start = tick();
        auto p_locations = sol_best.get_pLocations();

        for (auto loc:locations) { // First improvement over locations
            // if (!p_locations.contains(loc)) {
            if (std::find(p_locations.begin(), p_locations.end(), loc) == p_locations.end()){
                for (auto p_loc:p_locations) { // Best improvement over p_locations
                    if(test_Cover(p_loc, loc)){
                        sol_tmp = sol_best;
                        sol_tmp.replaceLocation(p_loc, loc);
                        
                        if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { 
                            sol_cand = sol_tmp;
                            improved = true;
                            // objectiveCpt = 0;
                        }
                        // else{
                        //     objectiveCpt++;

                        //     if(objectiveCpt == TOLERANCE_CPT){
                        //         break;
                        //     }
                        // }
                    }
                }
            }
            auto elapsed_time = duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count();
            if (improved) {

                sol_best = sol_cand;

                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the algorithm.\n";
                    printSolution_TB(sol_best, elapsed_time);
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    return sol_best;
                }

                break;
            }else{

                if (elapsed_time >= time_limit_seconds) {
                    cout << "Time limit reached. Stopping the uncapacitated TB  algorithm.\n";
                    printSolution_TB(sol_best, elapsed_time);
                    cout << "uncapacitated TB loop FINAL elapsed time: " << elapsed_time << " seconds\n";
                    if(sol_best.isSolutionFeasible() == false){
                        cout << "tb solution is not feasible\n";
                    }else{
                        cout << "tb solution is feasible\n";
                    }
                    return sol_best;
                }


            };
        }
        if (verbose) {
            printSolution_TB(sol_best, duration_cast<seconds>(high_resolution_clock::now() - start_time_total).count());
            cout << endl;
        }
        ite++;
    }

    cout << "Num ite total uncapacited TB: " << ite << "\n";
    if(sol_best.isSolutionFeasible() == false){
        cout << "tb solution is not feasible\n";
    }else{
        cout << "tb solution is feasible\n";
    }
    return sol_best;
}

bool TB::test_Capacity(Solution_cap sol, uint_t in_p, uint_t out_p) {
    // test if the new solution is feasible
    if (instance->getLocCapacity(out_p) <= 1) return false;
    if (sol.getTotalCapacity() - instance->getLocCapacity(in_p) + instance->getLocCapacity(out_p) < instance->getTotalDemand()) return false;
    return true;
}

int TB::isSolutionExistsinMap(Solution_cap sol, uint_t in_p, uint_t out_p) {
    // test if solution already exists in map
    unordered_set<uint_t> p_locations_test = sol.get_pLocations();
    p_locations_test.erase(in_p);
    p_locations_test.insert(out_p);
    
    int index = solutions_map.pSetExists_index(p_locations_test);
    if (index != -1){
        return index;
    }else{
        return -1;
    }
}

bool  TB::test_LB_PMP(Solution_cap sol, uint_t in_p, uint_t out_p) {
    // test if the new solution is feasible
    if (sol.getTotalCapacity() - instance->getLocCapacity(in_p) + instance->getLocCapacity(out_p) < instance->getTotalDemand()) return false;
    Solution_cap sol_tmp = sol;
    sol_tmp.replaceLocation(in_p, out_p, "PMP");
    if (sol.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { // LB1
        return true;
    }
 
    return false;
}

bool TB::test_Cover(uint_t in_p, uint_t out_p) {
    // test if the new solution is feasible
    if(!cover_mode) return true;
    if (instance->isInTheSameSubarea(in_p, out_p)) return true;    
    return false;
}

Solution_cap TB::localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE) {

    // cout << "TB local search capacitated started\n";
    
    string report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(sol_best.get_pLocations().size()) + ".csv";
    if(cover_mode) report_filename = "./reports/report_"+ this->typeMethod + "_" + instance->getTypeService() + "_p_" + to_string(sol_best.get_pLocations().size()) + "_cover_" + instance->getTypeSubarea() +".csv";

    //// time limit ////
    auto time_limit_seconds = CLOCK_LIMIT;
    ///
    auto locations = instance->getLocations();
    bool improved = true;
    Solution_cap sol_cand;

    if (generate_reports)
        writeReport_TB(report_filename, sol_best.get_objective(), 0, solutions_map.getNumSolutions(), external_time);


    int ite = 1;
    auto start_time_total = get_wall_time_TB();
    while (improved && ite < MAX_ITE) {
        
        improved = false;
        sol_cand = sol_best;
        auto p_locations = sol_best.get_pLocations();

        vector<uint_t> locations_not_in_p;
        for (auto loc : locations) 
            if (find(p_locations.begin(), p_locations.end(), loc) == p_locations.end())                 // loc is not in p_locations, so add it to locations_not_in_p
                locations_not_in_p.push_back(loc);
            
        auto start_time = get_wall_time_TB();
        for (auto loc:locations_not_in_p) { // First improvement over locations
            // #pragma omp parallel for{
            // #pragma omp parallel num_threads(4){
            for (auto p_loc:p_locations) { // Best improvement over p_locations
                Solution_cap sol_tmp = sol_best;    // N1 for sol_best
                if (test_Capacity(sol_tmp, p_loc, loc) && test_Cover(p_loc, loc)){ 
                    int index = isSolutionExistsinMap(sol_tmp, p_loc, loc);
                    if (index != -1){
                        if(solutions_map.getObjectiveByIndex(index)  < sol_cand.get_objective()){
                            sol_cand = solutions_map.getSolution(index);
                            improved = true;
                        }
                    }else if (test_LB_PMP(sol_tmp,p_loc,loc)) { // LB1
                        
                        // #pragma omp critical{
                        sol_tmp.replaceLocation(p_loc, loc, "GAPrelax");
                        // }
                        if(sol_tmp.isSolutionFeasible())
                            solutions_map.addUniqueSolution(sol_tmp);
                        // sol_tmp.replaceLocation(p_loc, loc, "heuristic");

                        auto elapsed_time = (get_wall_time_TB() - start_time) + external_time;
                        if (sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE_OBJ) { // LB2
                
                            if (verbose) {
                                cout << "Solution Improved: \n";
                                printSolution_TB(sol_tmp, elapsed_time);
                                cout << endl;
                            }
                            sol_cand = sol_tmp;
                            improved = true;

                            if (generate_reports)
                                writeReport_TB(report_filename, sol_cand.get_objective(), ite, solutions_map.getNumSolutions(),(get_wall_time_TB() - start_time_total) + external_time);

                        }
                    }
                }else if(!test_Capacity(sol_cand, p_loc, loc)){
                    // cout << "capacity constraint violated: yes\n";
                }
            }
            // }
        }

        auto elapsed_time_total = (get_wall_time_TB() - start_time_total) + external_time;
        if (improved) {
            sol_best = sol_cand;

            // auto p_loc = sol_cand.get_pLocations();
            // sol_best = Solution_cap(instance, p_loc,"GAPrelax", cover_mode);
            // solutions_map.addUniqueSolution(sol_best);

            if (verbose) {
                cout << "Solution Improved global TB: \n";
                printSolution_TB(sol_best, elapsed_time_total);
                cout << endl;
            }

            if (generate_reports)
                writeReport_TB(report_filename, sol_cand.get_objective(), ite, solutions_map.getNumSolutions(),elapsed_time_total);
            
            if (elapsed_time_total >= time_limit_seconds) {
                cout << "Time limit reached. Stopping the TB algorithm.\n";
                cout << "Num ite capacited TB: " << ite << "\n";
                cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time_total << " seconds\n";
                return sol_best;
            }

            ite++;
            break;
        }else{
            if (elapsed_time_total >= time_limit_seconds) {
                cout << "Time limit reached. Stopping the TB algorithm.\n";
                cout << "Num ite capacited TB: " << ite << "\n";
                cout << "capacitated TB loop FINAL elapsed time: " << elapsed_time_total << " seconds\n";
                return sol_best;
            }
        };
        
        if (verbose) {
            cout << "Solution Best: \n";
            printSolution_TB(sol_best, (get_wall_time_TB() - start_time_total) + external_time);
            cout << endl;
        }

        ite++;
    }

    cout << "Number of iterations capacited TB: " << ite << "\n";
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

void TB::setExternalTime(double time) {
    this->external_time = time;
}
void TB::setCoverMode(bool cover_mode) {
    this->cover_mode = cover_mode;
}