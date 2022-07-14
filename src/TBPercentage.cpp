#include "TBPercentage.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include <utility>

#include<chrono>
#include<thread>

TBPercentage::TBPercentage(shared_ptr<Instance> instance, uint_t seed):instance(std::move(instance)) {
    engine.seed(seed);
//    cout << "TB heuristic initialized\n";
//    instance->print();
}

Solution_std TBPercentage::initRandomSolution() {
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

//Solution_cap TBPercentage::initSetSolution();


Solution_cap TBPercentage::initRandomCapSolution() {
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

Solution_cap TBPercentage::initHighestCapSolution() {
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
    Solution_cap sol(instance, p_locations);

    return sol;
}



/**
 * @brief Splits the p locations into a stationary and a mobile part
 * 
 * @param movingAmount 
 * @param pLocations 
 * @return unordered_set<uint_t>* 
 */
unordered_set<uint_t>* TBPercentage::randomSplitLocationsByPercentage(int movingAmount, unordered_set<uint_t> pLocations){
    unordered_set<uint_t>* splitLocations = new unordered_set<uint_t>[2];
    unordered_set<uint_t> tempPLocations = pLocations;
    srand (time(NULL)); //Generate random seed
    
    while(splitLocations[0].size() < movingAmount){
        int loc_id = rand() % tempPLocations.size();
        auto sample = *std::next(tempPLocations.begin(), loc_id);
        tempPLocations.erase(sample);
        splitLocations[0].insert(sample);
    }
    splitLocations[1] = tempPLocations; // remaining locations are the stationary ones
    

    return splitLocations;
}


Solution_std TBPercentage::run(bool verbose) {
    checkClock();

    auto sol_best = initRandomSolution();
    sol_best.print();
    auto p_locations = sol_best.get_pLocations();
    auto locations = instance->getLocations();
    bool improved = false;
    bool cptBool = false;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    Solution_std sol_best_first;
    Solution_std sol_init = sol_best;
    int cpt = 0;
    int movingPercentage = 100 - PERCENTAGE;

    int p = sol_best.get_pLocations().size();
    int pToMove = p * movingPercentage / 100;
    unordered_set<uint_t>* splitLocations;
    unordered_set<uint_t> keptLocations;
    unordered_set<uint_t> movingLocations;

    //First Iteration
    checkClock();
    improved = false;
    sol_cand = sol_best;
    auto start = tick();
    //Splits the p locations into a stationary and a mobile part
    splitLocations = randomSplitLocationsByPercentage(pToMove, p_locations);
    keptLocations = splitLocations[0];
    movingLocations = splitLocations[1];
    for(auto loc:locations){
        if(!movingLocations.contains(loc)){
            for(auto loc_m:movingLocations){
                sol_tmp = sol_best;
                sol_tmp.replaceLocation(loc_m, loc);
                if(sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE){
                    movingLocations.erase(loc_m);
                    movingLocations.insert(loc);
                    sol_cand = sol_tmp;
                    improved = true;
                    cpt = 0;
                }
                else{
                    cpt++;
                    if(cpt == int(K/2)){
                        cptBool = true;
                        break;
                    }
                }
            }
        }
        if(improved){
            sol_best = sol_cand;
            improved = false;
            
            if(cptBool){
                cptBool = false;
                break;
            }
        }
    }
    if (verbose) {
        sol_best.print();
        cout << "TBPercentage loop: ";
        tock(start);
        cout << endl;

        sol_best_first = sol_best;
    }

    //Second Iteration
    std::cout << "second iteration" << std::endl;
    checkClock();
    improved = false;
    sol_cand = sol_best = sol_init;
    start = tick();

    splitLocations = randomSplitLocationsByPercentage(pToMove, p_locations);

    keptLocations = splitLocations[0];
    movingLocations = splitLocations[1];
    
    for(auto loc:locations){
        if(!movingLocations.contains(loc)){
            for(auto loc_m:movingLocations){
                sol_tmp = sol_best;
                sol_tmp.replaceLocation(loc_m, loc);
                if(sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE){
                    movingLocations.erase(loc_m);
                    movingLocations.insert(loc);
                    sol_cand = sol_tmp;
                    improved = true;
                    cpt = 0;
                    break;
                }
                else{
                    cpt++;
                    if(cpt == K){
                        cptBool = true;
                        break;
                    }
                }
                
            }
        }
        if(improved){
            sol_best = sol_cand;
            improved = false;
        }

        if(cptBool){
                break;
            }
    }
    if (verbose) {
        sol_best.print();
        cout << "TBPercentage loop: ";
        tock(start);
        cout << endl;
    }

    std::cout << "original p locations: " << std::endl;
    for(auto loc:p_locations){
        std::cout << loc << " ";
    }

    std::cout << std::endl;

    std::cout << "new p locations: " << std::endl;
    sol_best.print();

    

    if(sol_best.get_objective() < sol_best_first.get_objective()){
    checkClock();
    return sol_best;
}

    checkClock();
    return sol_best_first;
}

Solution_cap TBPercentage::run_cap(bool verbose) {
    checkClock();

    volatile bool flag = true;
    auto sol_best = initHighestCapSolution();
    auto locations = instance->getLocations();
    bool improved = true;
    Solution_cap sol_tmp;
    Solution_cap sol_cand;
    int cpt = 0;
    int movingPercentage = 100 - PERCENTAGE;

    int p = sol_best.get_pLocations().size();
    int pToMove = p * movingPercentage / 100;
    unordered_set<uint_t>* splitLocations = randomSplitLocationsByPercentage(pToMove, sol_best.get_pLocations());
    unordered_set<uint_t> keptLocations = splitLocations[0];
    unordered_set<uint_t> movingLocations = splitLocations[1];

    vector<uint_t> movingLocationsVector;
    movingLocationsVector.reserve(movingLocations.size());
    for(auto loc:movingLocations){
        movingLocationsVector.push_back(loc);
    }

    while(improved){
        //Mettre un gros cout dégueu pour lister toutes les locations immobiles
        checkClock();
        improved = false;
        sol_cand = sol_best;
        auto start = tick();
        //Splits the p locations into a stationary and a mobile part
        splitLocations = randomSplitLocationsByPercentage(pToMove, sol_best.get_pLocations());
        keptLocations = splitLocations[0];
        movingLocations = splitLocations[1];
        
        for(auto loc:locations){
            if(!movingLocations.contains(loc)){
                #pragma omp parallel for shared(flag)
                for(auto loc_m:movingLocationsVector){
                    if(flag) continue;
                    sol_tmp = sol_best;
                    if (sol_tmp.getTotalCapacity() - instance->getLocCapacity(loc_m) + instance->getLocCapacity(loc) >= instance->getTotalDemand()){
                        sol_cand.replaceLocation(loc_m, loc);
                        #pragma omp critical
                        if(sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE){
                            sol_cand = sol_tmp;
                            improved = true;
                            cpt = 0;
                            flag = true;
                        }
                        else{
                            cpt++;

                            if(cpt == K/2){
                                //Mettre un gros cout qui liste toute les locations, pour vérif si on a bien les immobiles qui ont pas bougées
                                flag = false;
                            }
                        }
                    }
                    
                }
            }
            if(improved){
                sol_best = sol_cand;
                break;
            }
        }
        if (verbose) {
            sol_best.print();
            cout << "TBCapPercentage loop: ";
            tock(start);
            cout << endl;
        }
    }
    checkClock();

    return sol_best;
}

