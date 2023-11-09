#ifndef PMP_HPP
#define PMP_HPP
#include <iostream>
#include <filesystem>
#include <sys/time.h>

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
        PMP(const shared_ptr<Instance>& instance);
        ~PMP();
        void exportILP      (IloCplex& cplex);
        void solveILP       (void);
        // void printSolution  (IloCplex& cplex,
        //                     BoolVarMatrix x,
        //                     IloBoolVarArray y);
        // void saveSolution   (IloCplex& cplex,
        //                     BoolVarMatrix x,
        //                     IloBoolVarArray y);
        // // void saveResults    (IloCplex& cplex=this->cplex,
        // //                     double timeF=0.0);
        // void saveResults    (double timeF=0.0);
        // void printInstance  (void);
        // int current_day, current_month, current_year;
        // void saveNumConstraints();
        float timePMP;

    private:
        shared_ptr<Instance> instance; // original PMP instance
        IloEnv env;
        IloModel model;
        IloCplex cplex;
        BoolVarMatrix x;
        IloBoolVarArray y;

        // void allocVars(env, x, y);

        // void initILP        (const char* typeProb);
        // void initVariables  (void);
        // void createModel    (IloModel model,
        //                     BoolVarMatrix x,
        //                     IloBoolVarArray y);
        
        // void createModel(const char* typeProb,
        //                 IloModel model, 
        //                 BoolVarMatrix x,
        //                 IloBoolVarArray y);
        
        // void
        // objFunction (IloModel model, BoolVarMatrix x);

        // void
        // constr_DemandSatif (IloModel model, BoolVarMatrix x);

        // void
        // constr_pLocations (IloModel model, IloBoolVarArray y);

        // void
        // constr_maxCapacity (IloModel model, BoolVarMatrix x, IloBoolVarArray y);

        

};



#endif // PMP_H