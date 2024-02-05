#ifndef PMP_HPP
#define PMP_HPP
#include <iostream>
#include <experimental/filesystem>
#include <sys/time.h>
#include <string.h>
#include <utility>

#include "instance.hpp"
#include "globals.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "utils.hpp"
#include "TB.hpp"

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
        void run           (void);
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
        


    private:
        shared_ptr<Instance> instance; // original PMP instance
        const char* typeProb;
        IloEnv env;
        IloModel model;
        IloCplex cplex;
        BoolVarMatrix x_bin;
        IloBoolVarArray y;
        NumVarMatrix x_cont;
        bool isFeasible_Solver=false;
        // bool isFeasible=false;
        // IloNumVarArray y_cont;
        uint_t p;
        uint_t num_facilities;
        uint_t num_customers;

        void initVars();
        void initILP        (void);


        template <typename VarType>        
        void createModel(IloModel model, 
                        VarType x,
                        IloBoolVarArray y);
        
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

};



#endif // PMP_H