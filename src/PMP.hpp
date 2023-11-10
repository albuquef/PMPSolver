#ifndef PMP_HPP
#define PMP_HPP
#include <iostream>
#include <filesystem>
#include <sys/time.h>
#include <string.h>

#include "instance.hpp"
#include "instance.hpp"
#include "solution_std.hpp"

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
        PMP(const shared_ptr<Instance>& instance, const char* typeProb);
        ~PMP();
        void exportILP      (IloCplex& cplex);
        void solveILP       (void);
        void printSolution  (IloCplex& cplex,
                            BoolVarMatrix x,
                            IloBoolVarArray y);
        // void saveSolution   (IloCplex& cplex,
        //                     BoolVarMatrix x,
        //                     IloBoolVarArray y);
        // void saveResults    (IloCplex& cplex=this->cplex,
        //                     double timeF=0.0);
        // void saveResults    (double timeF=0.0);
        // void printInstance  (void);
        int current_day, current_month, current_year;
        // void saveNumConstraints();
        double timePMP;

    private:
        shared_ptr<Instance> instance; // original PMP instance
        const char* typeProb;
        IloEnv env;
        IloModel model;
        IloCplex cplex;
        BoolVarMatrix x;
        IloBoolVarArray y;
        uint_t p;
        uint_t num_facilities;
        uint_t num_customers;

        void allocVars(IloEnv env, BoolVarMatrix x, IloBoolVarArray y);

        void initILP        (void);
        void initVariables  (void);        
        void createModel(IloModel model, 
                        BoolVarMatrix x,
                        IloBoolVarArray y);
        
        void
        objFunction (IloModel model, BoolVarMatrix x);

        void
        constr_DemandSatif (IloModel model, BoolVarMatrix x);

        void
        constr_pLocations (IloModel model, IloBoolVarArray y);

        void
        constr_maxCapacity (IloModel model, BoolVarMatrix x, IloBoolVarArray y);

        

};



#endif // PMP_H