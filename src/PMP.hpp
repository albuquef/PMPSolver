#ifndef PMP_HPP
#define PMP_HPP
#include <iostream>
#include <experimental/filesystem>
#include <sys/time.h>
#include <chrono>
#include <string.h>
#include <utility>
#include <cmath>
#include <mutex>    

#include "instance.hpp"
#include "globals.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "utils.hpp"
// #include "TB.hpp"

#include <ilcplex/ilocplex.h>

using namespace std;

//double runTime;
#define BILLION 1000000000L

ILOSTLBEGIN

//typedef
typedef IloArray<IloIntVarArray> IntVarMatrix;
typedef IloArray<IloBoolVarArray> BoolVarMatrix;
typedef IloArray<BoolVarMatrix> BoolVar3Matrix;
typedef IloArray<BoolVar3Matrix> BoolVar4Matrix;

typedef IloArray<IloNumVarArray> NumVarMatrix;
typedef IloArray<NumVarMatrix> NumVar3Matrix;




class PMP
{

    public: 
        // PMP(Reader *r, const char* typeSEC):rd(r){rd->show();};
        PMP(const shared_ptr<Instance>& instance,const char* typeProb, bool is_BinModel=false);
        ~PMP();
        void exportILP      (IloCplex& cplex);
        void solveILP       (void);
        void run           (string Method_name="void");
        void run_GAP       (unordered_set<uint_t> p_locations);
        template <typename VarType>  
        void printSolution  (IloCplex& cplex,
                            VarType x,
                            IloBoolVarArray y);
        Solution_std getSolution_std(void);
        Solution_cap getSolution_cap(void);
        void setSolution_cap(Solution_cap sol);
        void saveVars   (const string& filename,const string& Method);
        void saveResults(const string& filename,const string& Method);
        int current_day, current_month, current_year;
        // void saveNumConstraints();
        double timeSolver;
        double timePMP;
        bool is_BinModel;
        bool VERBOSE;
        string typeServ;
        bool getFeasibility_Solver();
        void setGenerateReports(bool add_generate_reports);
        void setAddBreakCallback(bool add_break_callback);
        // void setVerbose(bool VERBOSE);
        bool CoverModel=false;
        bool CoverModel_n2=false;
        string typeSubarea="null";
        string typeSubarea_n2="null";
        void setCoverModel(bool CoverModel, string typeSubarea);
        void setCoverModel_n2(bool CoverModel_n2, string typeSubarea_n2);
        void setUpperBound(double UpperBound);
        void setTimeLimit(double timeLimit);
        void setMIPStartSolution(Solution_cap sol);
        void setBestBound(dist_t BestBound);
        void setDisplayCPLEX(bool displayCPLEX);

        // void setInitialSolution(Solution_cap sol);
        
        // // Method to retrieve CPLEX object
        // IloCplex getCplex() const;

        // // Methods to retrieve variables
        // BoolVarMatrix getX_bin() const;
        // IloBoolVarArray getY() const;

        void set_pLocations_from_solution(unordered_set<uint_t> p_locations);
        void set_MaxNeighbors_from_solution(uint_t MaxNeighbors);
        void set_Fixed_pLocations_from_solution(unordered_set<uint_t> p_locations_fixed);

        void set_PriorityListLocations(vector<uint_t> priorityLocations, string priorityStrategy);
        void setPriorityStrategy(string priorityStrategy);
        void setCutsType(string cuts_type);

    private:
        // mutex callbackMutex;
        shared_ptr<Instance> instance; // original PMP instance
        const char* typeProb;
        IloEnv env;
        IloModel model;
        IloCplex cplex;
        BoolVarMatrix x_bin;
        IloBoolVarArray y;
        NumVarMatrix x_cont;
        bool isFeasible_Solver=false;
        bool add_generate_reports=false;
        bool add_break_callback=false;
        double UpperBound=0;
        dist_t BestBound=0;
        // IloNumVarArray y_cont;
        uint_t p;
        uint_t num_facilities;
        uint_t num_customers;
        uint_t num_subareas;
        double timeLimit = CLOCK_LIMIT_CPLEX;
        bool useMIPStart=false;
        Solution_cap initial_solution;
        void set_gap_report_filename(string Method_name);
        bool displayCPLEX=true;
        string priorityStrategy = "index_based";  // "index_based" or "presence_based"
        string cuts_type="none"; // 

        void initVars();
        void initILP        (void);


        template <typename VarType>        
        void createModel(IloModel model, 
                        VarType x,
                        IloBoolVarArray y);

        vector<uint_t> priorityVoteLocations;        
        void createPriorityListLocations(IloModel model, IloBoolVarArray y);

        template <typename VarType>
        void objFunction (IloModel model, VarType x);
        // void objFunction (IloModel model, BoolVarMatrix x);
        
        
        // void constr_DemandSatif (IloModel model, BoolVarMatrix x);
        template <typename VarType>
        void constr_DemandSatif (IloModel model, VarType x);
        
        void constr_pLocations (IloModel model, IloBoolVarArray y);

        // for pmp model 
        template <typename VarType>
        void constr_UBpmp (IloModel model, VarType x, IloBoolVarArray y);
        
        // void constr_maxCapacity (IloModel model, BoolVarMatrix x, IloBoolVarArray y);
        template <typename VarType>
        void constr_maxCapacity (IloModel model, VarType x, IloBoolVarArray y);

        unordered_set<uint_t> p_locations; // p selected locations
        void constr_GAP (IloModel model, IloBoolVarArray y);

        void constr_Cover (IloModel model, IloBoolVarArray y);
        void constr_Cover_n2 (IloModel model, IloBoolVarArray y);


        template <typename VarType>
        void constr_UpperBound (IloModel model, VarType x);

        void addMIPStartSolution();

        template <typename VarType>
        void constr_MaxDistance (IloModel model, VarType x);

        template <typename VarType>
        void constr_MaxAssignments (IloModel model, VarType x);


        bool add_constr_maxNeighbors_from_solution = false;
        uint_t MaxNeighbors_with_solution = 0;
        unordered_set<uint_t> p_locations_from_solution; // p selected locations from  a solution
        void constr_MaxNeighborsFromSolution (IloModel model, IloBoolVarArray y);

        bool add_constr_FixedAllocs_from_solution = false;
        unordered_set<uint_t> fixed_p_locations; // p selected locations from  a solution
        template <typename VarType>
        void constr_fixedAllocs_from_solution (IloModel model, IloBoolVarArray y, VarType x);

        template <typename VarType>
        void cut_UBvarX_closestj (IloModel model, VarType x, IloBoolVarArray y);

};




#endif // PMP_H