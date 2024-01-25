#ifndef SOLUTION_MAP_H
#define SOLUTION_MAP_H

#include <vector>
#include "solution_cap.hpp"

class Solution_MAP {
private:
    shared_ptr<Instance> instance;
    std::vector<Solution_cap> solutions;

    // Compare two sets for equality regardless of their ordering
    // bool areSetsEqual(const std::unordered_set<uint_t>& set1, const std::unordered_set<uint_t>& set2) const {
    //     return set1.size() == set2.size() && std::equal(set1.begin(), set1.end(), set2.begin());
    // }
    bool areSetsEqual(const std::unordered_set<uint_t>& set1, const std::unordered_set<uint_t>& set2) const {
        return set1 == set2;
    }
     // Add a solution 
    void addSolution(const Solution_cap& solution) {
        solutions.push_back(solution);
    }

public:
    Solution_MAP() = default;
    Solution_MAP(shared_ptr<Instance> instance) : instance(std::move(instance)) {};

    // Add a solution 
    // void addSolution(const Solution_cap& solution) {
    //     solutions.push_back(solution);
    // }

    bool addUniqueSolution(const Solution_cap& newSolution) {
        if(newSolution.get_objective() == 0 || newSolution.get_pLocations().size() == -1){
            return false;
        }
        // if(newSolution.get_pLocations().size() == 0 || newSolution.get_pLocations().size() != instance->get_p()){
        //     return false;
        // }
        
        if (!solutionExists(newSolution)) {
            addSolution(newSolution);
            return true;
        }else{
            cout<<"Solution already exists"<<endl;
            return false;   
        }
    }

    // Check if a solution with the same p_locations already exists
    bool solutionExists(const Solution_cap& newSolution) const {
        for (const auto& solution : solutions) {
            if (areSetsEqual(solution.get_pLocations(), newSolution.get_pLocations())) {
                return true;
            }
        }
        return false;
    }

    bool pSetExists(const unordered_set<uint_t>& p_locations) const {
        for (const auto& solution : solutions) {
            if (areSetsEqual(solution.get_pLocations(), p_locations)) {
                return true;
            }
        }
        return false;
    }

    int pSetExists_index(const std::unordered_set<uint_t>& p_locations) const {
        for (size_t i = 0; i < solutions.size(); ++i) {
            if (areSetsEqual(solutions[i].get_pLocations(), p_locations)) {
                return static_cast<int>(i);  // Return the index when a match is found
            }
        }
        return -1;  // Return -1 if no match is found
    }

    // Get the number of stored solutions
    size_t getNumSolutions() const {
        return solutions.size();
    }

    // Get a specific solution by index
    const Solution_cap& getSolution(size_t index) const {
        if (index < solutions.size()) {
            return solutions[index];
        } else {
            // Handle out-of-range index
            throw std::out_of_range("Index out of range");
        }
    }

    dist_t getObjectiveByIndex(int index) const {
        if (index >= 0 && static_cast<size_t>(index) < solutions.size()) {
            return solutions[index].get_objective();
        } else {
            // Handle invalid index, you might want to throw an exception or return a default value
            std::cerr << "Invalid index: " << index << std::endl;
            return -1.0;  // Return a default value, adjust as needed
        }
    }


    // For example, a method to print all stored solutions
    void printAllSolutions() {
        for (auto& solution : solutions) {
            solution.print();
        }
    }

    // Add more methods as needed based on your requirements
};

#endif // SOLUTION_MAP_H