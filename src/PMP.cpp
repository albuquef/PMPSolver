#include "PMP.hpp"

double get_wall_time(){
    struct timeval time;
    if(gettimeofday(&time,nullptr)){
        // HANDLE ERROR
        return 0;
    }else{
        return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_usec*0.000001); //microsegundos
    }
}


PMP::PMP(const shared_ptr<Instance>& instance,const char* typeProb, bool is_BinModel):instance(instance)
{
    // this->instance = instance;
    this->typeProb = typeProb;
    this->is_BinModel = is_BinModel;
    this->p = this->instance->get_p();
    this->num_facilities = this->instance->getLocations().size();
    this->num_customers = this->instance->getCustomers().size();

    cout << "Value of p: " << this->p << endl;
    cout << "Number of facilities: " << num_facilities << endl;
    cout << "Number of customers: " << num_customers << endl;
    // cout << "Type of problem: " << typeProb << endl;
    if (strcmp(typeProb,"CPMP") == 0 || strcmp(typeProb,"cPMP") == 0  )
        cout << "Capacity Model: true" << endl;
    else 
        cout << "Capacity Model: false" << endl;
    if (is_BinModel == true) 
        cout << "Binary Model: true" << endl;
    else 
        cout << "Binary Model: false" << endl;

    initILP();
    solveILP();
    
    if (cplex.getStatus() == IloAlgorithm::Optimal)
        if(is_BinModel == true) {printSolution(cplex,x_bin,y);}
        else {printSolution(cplex,x_cont,y);}
    else
        cout << "Solution status = " << cplex.getStatus()   << endl;

}
PMP::~PMP()
{
    // env.end();
}

void PMP::initVars(){

    IloEnv env = model.getEnv();

    // alloc memory and add to model for vars y_j
    y = IloBoolVarArray(env, static_cast<IloInt>(num_facilities));
    for(IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
        char name[50];
        sprintf(name, "y(%ld)", j+1);
        this->y[j].setName(name);
        model.add(this->y[j]);
    }



    // alloc memory for vars x_ij and add to model
    if(is_BinModel == true){
        this->x_bin = BoolVarMatrix(env, static_cast<IloInt>(num_customers));
        for (IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
            this->x_bin[i] = IloBoolVarArray(env, static_cast<IloInt>(num_facilities));
        
            for (IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
                for (IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
                    char name[50];
                    sprintf(name, "x(%ld,%ld)", i+1, j+1);
                    this->x_bin[i][j].setName(name);
                    model.add(this->x_bin[i][j]);
                }
    }else{
        this->x_cont = NumVarMatrix(env, static_cast<IloInt>(num_customers));
        for (IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
            this->x_cont[i] = IloNumVarArray(env, static_cast<IloInt>(num_facilities),0.0,1.0, ILOFLOAT);

            for (IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
                for (IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
                    char name[50];
                    sprintf(name, "x(%ld,%ld)", i+1, j+1);
                    this->x_cont[i][j].setName(name);
                    model.add(this->x_cont[i][j]);
                }
    }

}

void PMP::initILP(){



    try{

        model = IloModel(env);
        initVars();

        if(is_BinModel == true)
            createModel(this->model,this->x_bin,this->y);
        else
            createModel(this->model,this->x_cont,this->y);     

        this->cplex = IloCplex(this->model);
        exportILP(cplex);
   
        cplex.setParam(IloCplex::TiLim, CLOCK_LIMIT); // time limit CLOCK_LIMIT seconds
        // cplex.setParam(IloCplex::TreLim, 30000); // tree memory limit 30GB
        cplex.setParam(IloCplex::Threads, 8); // use 8 threads

    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\nError ilocplex" << endl;
        return;
    }catch (int e) {
            cerr << endl << "\nException occurred = " << e << endl;
    }


}


// void PMP::createModel(IloModel model, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType> 
void PMP::createModel(IloModel model, VarType x, IloBoolVarArray y){    

    objFunction(model,x);
    constr_DemandSatif(model,x);
    constr_pLocations(model,y);
    if(strcmp(typeProb,"CPMP") == 0 || strcmp(typeProb,"cPMP") == 0  ){constr_maxCapacity(model,x,y);}
    if(strcmp(typeProb,"PMP") == 0 || strcmp(typeProb,"pmp") == 0  ){constr_UBpmp(model,x,y);}

}


// void PMP::objFunction(IloModel model, BoolVarMatrix x){
template <typename VarType>
void PMP::objFunction(IloModel model, VarType x){   

    cout << "[INFO] Adding Objective Function "<< endl;
    
    IloEnv env = model.getEnv();
    IloExpr objExpr(env);
    for (IloInt i = 0; i < num_customers; i++)
        for (IloInt j = 0; j < num_facilities; j++){
            // if(strcmp(typeProb,"PMP") == 0 || strcmp(typeProb,"pmp") == 0  ){objExpr += instance->getRealDist(j+1,i+1) * x[i][j];}
            // else{objExpr += instance->getWeightedDist(j+1,i+1) * x[i][j];}
            auto loc = instance->getLocations()[j];
            auto cust = instance->getCustomers()[i];
            objExpr += instance->getWeightedDist(loc,cust) * x[i][j];
        }
    model.add(IloMinimize(env, objExpr));
    objExpr.end();
}

// void PMP::constr_DemandSatif(IloModel model, BoolVarMatrix x){
template <typename VarType>
void PMP::constr_DemandSatif(IloModel model, VarType x){

    cout << "[INFO] Adding Demand Satisfied Constraints "<< endl;

    IloEnv env = model.getEnv();
    for (IloInt i = 0; i < num_customers; i++){
        IloExpr expr(env);
        for (IloInt j = 0; j < num_facilities; j++){
            expr += x[i][j];
        }
        model.add(expr == 1);
        expr.end();
    }

}


void PMP::constr_pLocations(IloModel model, IloBoolVarArray y){

    cout << "[INFO] Adding p Locations Constraints "<< endl;

    IloEnv env = model.getEnv();
    IloExpr expr(env);
    for (IloInt j = 0; j < num_facilities; j++)
        expr += y[j];
    model.add(expr == p);
    expr.end();

}

template <typename VarType>
void PMP::constr_UBpmp(IloModel model, VarType x, IloBoolVarArray y){

    cout << "[INFO] Adding UB Constraints "<< endl;

    IloEnv env = model.getEnv();
    for (IloInt i = 0; i < num_customers; i++)
        for (IloInt j = 0; j < num_facilities; j++)
            model.add(x[i][j] <= y[j]);

}


// void  PMP::constr_maxCapacity(IloModel model, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType>
void  PMP::constr_maxCapacity(IloModel model, VarType x, IloBoolVarArray y){


    cout << "[INFO] Adding Max Capacity Constraints "<< endl;

    IloEnv env = model.getEnv();
    for (IloInt j = 0; j < num_facilities; j++){
        IloExpr expr(env);
        auto loc = instance->getLocations()[j];
        for (IloInt i = 0; i < num_customers; i++){
            auto cust = instance->getCustomers()[i];
            expr += IloNum(instance->getCustWeight(cust)) * x[i][j];
        }
        model.add(expr <= IloNum(instance->getLocCapacity(loc)) * y[j]);
        expr.end();
    }

}

// void PMP::printSolution(IloCplex& cplex, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType>  
void PMP::printSolution(IloCplex& cplex, VarType x, IloBoolVarArray y){
    
        cout << "Solution status = " << cplex.getStatus()   << endl;
        cout << "Solution value  = " << cplex.getObjValue() << endl;
        double objectiveValue = cplex.getObjValue();
        cout << "Objective Value: " << fixed << setprecision(3) << objectiveValue << endl;
        cout << "Time to solve: " << timePMP << endl;

        // for (IloInt j = 0; j < num_facilities; j++){
        //     auto loc = instance->getLocations()[j];
        //     for (IloInt i = 0; i < num_customers; i++){
        //         auto cust = instance->getCustomers()[i];
        //         if (cplex.getValue(x[i][j]) > 0.001)
        //             cout << "x[" << cust << "][" << loc << "] = " << cplex.getValue(x[i][j]) << endl;
        //     }
        // }
    
        // for (IloInt j = 0; j < num_facilities; j++){
        //     auto loc = instance->getLocations()[j];
        //     if (cplex.getValue(y[j]) > 0.5)
        //         cout << "y[" << loc << "] = " << cplex.getValue(y[j]) << endl;
        // }


        cout << "Time total: " << cplex.getTime() << endl;
}


void PMP::exportILP(IloCplex& cplex)
{
    cplex.exportModel("model.lp");
}

void PMP::solveILP(){
    
    double cpu0, cpu1;
    cpu0 = get_wall_time(); 
    if (!cplex.solve()){
        env.error() << "Failed to optimize LP." << endl;
        // cout << "Solution status = " << cplex.getStatus()   << endl;
        // throw(-1);
    }
    cpu1 = get_wall_time();
    this->timePMP = cpu1 - cpu0; 
}


// Solution_std PMP::getSolution_std(){


// }

Solution_cap PMP::getSolution_cap(){
    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    for (IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
        if (cplex.getValue(y[j]) > 0.5)
            p_locations.insert(loc);
    }

    unordered_map<uint_t, dist_t> loc_usages; // p location -> usage from <0, capacity>
    unordered_map<uint_t, dist_t> cust_satisfactions; // customer -> satisfaction from <0, weight>
    unordered_map<uint_t, assignment> assignments; // customer -> assignment (p location, usage, weighted distance)

    for (auto p_loc:p_locations) loc_usages[p_loc] = 0;
    for (auto cust:instance->getCustomers()) {
        cust_satisfactions[cust] = 0;
        assignments[cust] = assignment{};
    }

    cout << "p_loc = ";
    for (auto p_loc:p_locations)
        cout << p_loc << ", ";
    cout << endl;   

    IloNum objtest = 0;

    for (IloInt j = 0; j < num_facilities; j++)
        if (cplex.getValue(y[j]) > 0.5){
            auto loc = instance->getLocations()[j];
            // cout << "loc = " << loc << endl;
            for (IloInt i = 0; i < num_customers; i++){
                auto cust = instance->getCustomers()[i];
                // cout << "cust = " << cust << endl;
                if (is_BinModel && cplex.getValue(x_bin[i][j]) > 0.001){
                    auto dem_used = cplex.getValue(x_bin[i][j])* instance->getCustWeight(cust);//instance->getWeightedDist(loc,cust);
                    loc_usages[loc] += dem_used;
                    cust_satisfactions[cust] += dem_used;
                    auto obj_increment = dem_used * instance->getRealDist(loc, cust);
                    assignments[cust].emplace_back(my_tuple{loc, dem_used, obj_increment});
                }else if (!is_BinModel && cplex.getValue(x_cont[i][j]) > 0.001){
                    auto dem_used = cplex.getValue(x_cont[i][j])* instance->getCustWeight(cust);
                    loc_usages[loc] += dem_used;
                    cust_satisfactions[cust] += dem_used;
                    auto obj_increment = dem_used * instance->getRealDist(loc, cust);
                    assignments[cust].emplace_back(my_tuple{loc, dem_used, obj_increment});
                    objtest += obj_increment;
                }


                if(loc_usages[loc] >= instance->getLocCapacity(loc) + 0.001){
                        
                        // cout << "loc = " << loc << endl;
                        // cout  << "loc_usages[loc] = " << loc_usages[loc] << endl;
                        // cout << "getLocCapacity(loc) = " << instance->getLocCapacity(loc) << endl;
                        cerr << "ERROR: usage > capacity" << endl;
                        
                        exit(1);
                }
                if(cust_satisfactions[cust] >= instance->getCustWeight(cust) + 0.001 ){

                    // cout << "cust = " << cust << endl;
                    // cout  << "cust_satisfactions[cust] = " << cust_satisfactions[cust] << endl;
                    // cout << "getCustWeight(cust) = " << instance->getCustWeight(cust) << endl;
                    cerr << "ERROR: satisfaction > weight" << endl;
                    exit(1);
                }

                
            }
        }
    cout << "test objetive value: " <<  objtest << endl;

    Solution_cap sol(instance, p_locations, loc_usages, cust_satisfactions, assignments);

    return sol;
}



Solution_std PMP::getSolution_std(){

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    auto locations = instance->getLocations();

    for (IloInt j = 0; j < num_facilities; j++){
        if (cplex.getValue(y[j]) > 0.5){
            p_locations.insert(locations[j+1]);
            // cout << "test y[" << j+1 << "] = " << cplex.getValue(y[j]) << endl;
            // cout << "test locations[" << j+1 << "] = " << locations[j+1] << endl;
        }
    }

    // cout << "locations[0] = " << locations[0] << endl;
    // cout << "locations[1] = " << locations[1] << endl;
    // cout << "locations[2] = " << locations[2] << endl;


    // cout << "get real dist 1 e 690: " << instance->getRealDist(690,1) << endl;
    // cout << "get real dist 534 e 580: " << instance->getRealDist(534,580) << endl;
    // cout << "get real dist 536 e 583: " << instance->getRealDist(536,583) << endl;



    Solution_std sol(instance, p_locations);

    return sol;
}






