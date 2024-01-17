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
        void saveVars   (const string& filename,const string& Method);
        void saveResults(const string& filename,const string& Method);
        int current_day, current_month, current_year;
        // void saveNumConstraints();
        double timeSolver;
        double timePMP;
        bool is_BinModel;
        bool VERBOSE;
        string typeServ;
        


    private:
        shared_ptr<Instance> instance; // original PMP instance
        const char* typeProb;
        IloEnv env;
        IloModel model;
        IloCplex cplex;
        BoolVarMatrix x_bin;
        IloBoolVarArray y;
        NumVarMatrix x_cont;
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

// Define the callback class
class GapInfoCallback : public IloCplex::MIPInfoCallbackI {
public:
    GapInfoCallback(IloEnv env, IloCplex cplex, IloNum startTime, IloNum lastPrintTime, IloNum lastBestBound, const std::string& filename)
        : IloCplex::MIPInfoCallbackI(env), cplex(cplex), startTime(startTime), lastPrintTime(lastPrintTime), lastBestBound(lastBestBound), filename(filename) {}

protected:
    void main() override {
        try {

        // cout << "Callback called" << endl;
        double interval_time = 5.0; // seconds

        if (cplex.getCplexTime() - lastPrintTime >= interval_time) {

            ofstream outputTable;
            outputTable.open("./reports/"+filename,ios:: app);

            
            if (!outputTable.is_open()) {
                // cerr << "Error opening file: " << output_filename << endl;
                cerr << "Error opening file: " << endl;
                // return;
            }else{
                outputTable << fixed << setprecision(15) << cplex.getObjValue() << ";"; // obj value
                outputTable << fixed << setprecision(15) << getBestObjValue() << ";"; // obj value
                outputTable << fixed << setprecision(15) << getIncumbentObjValue() << ";"; // obj value
                outputTable << getNnodes() << ";"; // num nodes
                outputTable << getMIPRelativeGap() <<";"; // relative gap
                outputTable << cplex.getCplexTime() - startTime <<  ";"; // time cplex
                outputTable << "\n";
            }
            outputTable.close();

            std::cout << "Time: " << cplex.getCplexTime() - startTime << " seconds" << std::endl;
            std::cout << "MIP Gap: " << getMIPRelativeGap() << std::endl;
            std::cout << "Nodes: " << getNnodes() << std::endl;
            std::cout << "Best Integer: " <<  fixed << setprecision(15) << cplex.getObjValue() << std::endl;
            std::cout << "Best Objective: " <<  fixed << setprecision(15) << getBestObjValue() << std::endl;
            std::cout << "Incumbent Obj:  " << fixed << setprecision(15) << getIncumbentObjValue() << std::endl;

            lastPrintTime = cplex.getCplexTime();
            lastBestBound = getBestObjValue();
        }
    } catch (IloException &ex) {
        std::cerr << "Error in callback function: " << ex.getMessage() << std::endl;
        throw; // Rethrow the exception to terminate the program
    }
    }

private:
    IloCplex cplex;
    IloNum startTime;
    IloNum lastPrintTime;
    IloNum lastBestBound;
    std::string filename;
};

#endif // PMP_H