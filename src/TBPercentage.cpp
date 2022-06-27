#include "TBPercentage.hpp"
#include "globals.hpp"
#include "utils.hpp"

#include <utility>

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
unordered_set<uint_t>* TBPercentage::splitLocationsByPercentage(int movingAmount, unordered_set<uint_t> pLocations){
    unordered_set<uint_t>* splitLocations = new unordered_set<uint_t>[2];
    unordered_set<uint_t> tempPLocations = pLocations;
    
    while(splitLocations[0].size() < movingAmount){
        uniform_int_distribution<uint_t> distribution (0, tempPLocations.size() - 1);
        auto loc_id = distribution(engine);
        auto test = *std::next(tempPLocations.begin(), loc_id);
        tempPLocations.erase(test);
        splitLocations[0].insert(test);
    }

    splitLocations[1] = tempPLocations; // remaining locations are the stationary ones


    
    return splitLocations;
}


Solution_std TBPercentage::run(bool verbose) {
    checkClock();

    auto sol_best = initRandomSolution();
    auto p_locations = sol_best.get_pLocations();
    auto locations = instance->getLocations();
    bool improved = true;
    Solution_std sol_tmp;
    Solution_std sol_cand;
    int cpt = 0;
    int movingPercentage = 100 - PERCENTAGE;

    int p = sol_best.get_pLocations().size();
    int pToMove = p * movingPercentage / 100;
    unordered_set<uint_t>* splitLocations;
    unordered_set<uint_t> keptLocations;
    unordered_set<uint_t> movingLocations;

    while(improved){
        checkClock();
        improved = false;
        sol_cand = sol_best;
        auto start = tick();
        //Splits the p locations into a stationary and a mobile part
        splitLocations = splitLocationsByPercentage(pToMove, sol_best.get_pLocations());
        keptLocations = splitLocations[0];
        movingLocations = splitLocations[1];
        
        for(auto loc:locations){
            if(!movingLocations.contains(loc)){
                for(auto loc_m:movingLocations){
                    sol_tmp = sol_best;
                    sol_tmp.replaceLocation(loc_m, loc);
                    if(sol_cand.get_objective() - sol_tmp.get_objective() > TOLERANCE){
                        sol_cand = sol_tmp;
                        improved = true;
                        cpt = 0;
                    }
                    else{
                        cpt++;
                        if(cpt == K/2){
                            //Mettre un gros cout qui liste toute les locations, pour vérif si on a bien les immobiles qui ont pas bougées
                            break;
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
            cout << "TBPercentage loop: ";
            tock(start);
            cout << endl;
        }
    }
    checkClock();

    return sol_best;
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
    unordered_set<uint_t>* splitLocations = splitLocationsByPercentage(pToMove, sol_best.get_pLocations());
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
        splitLocations = splitLocationsByPercentage(pToMove, sol_best.get_pLocations());
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

