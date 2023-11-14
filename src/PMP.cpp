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


PMP::PMP(const shared_ptr<Instance>& instance,const char* typeProb):instance(instance)
{
    this->instance = instance;
    this->typeProb = typeProb;
    this->instance->print();
    this->p = this->instance->get_p();
    this->num_facilities = this->instance->getLocations().size();
    this->num_customers = this->instance->getCustomers().size();

    cout << "value of p: " << this->p << endl;
    cout << "Number of facilities: " << num_facilities << endl;
    cout << "Number of customers: " << num_customers << endl;

    initILP();
    solveILP();
    printSolution(cplex,x,y);
}
PMP::~PMP()
{
    // env.end();
}

void PMP::initVars(){

    IloEnv env = model.getEnv();
    // alloc memory for vars x_ij and y_j
    this->x = BoolVarMatrix(env, static_cast<IloInt>(num_customers));
    for (IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
        this->x[i] = IloBoolVarArray(env, static_cast<IloInt>(num_facilities));
    



    // for (IloInt i = 0; i < num_customers; i++) {
    //         x[i] = IloBoolVarArray(env, num_facilities);
    //         for (IloInt j = 0; j < num_facilities; j++) {
    //             x[i][j] = IloBoolVar(env);
    //         }
    //     }


    y = IloBoolVarArray(env, static_cast<IloInt>(num_facilities));



    // add x_ij and y_j as binary variables
    for (IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
        for (IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
            char name[50];
            sprintf(name, "x(%ld,%ld)", i+1, j+1);
            this->x[i][j].setName(name);
            model.add(this->x[i][j]);
        }
    for(IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
        char name[50];
        sprintf(name, "y(%ld)", j+1);
        this->y[j].setName(name);
        model.add(this->y[j]);
    }

}

void PMP::initILP(){



    try{

        model = IloModel(env);
        initVars();     
        createModel(model,this->x,this->y);

        this->cplex = IloCplex(this->model);
        // exportILP(cplex);
   
        cplex.setParam(IloCplex::TiLim, CLOCK_LIMIT); // time limit CLOCK_LIMIT seconds
        // cplex.setParam(IloCplex::TreLim, 30000); // tree memory limit 30GB

        // exportILP(cplex);

    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\nError ilocplex" << endl;
        return;
    }catch (int e) {
            cerr << endl << "\nException occurred = " << e << endl;
    }


}

void PMP::createModel(IloModel model, BoolVarMatrix x, IloBoolVarArray y){


    // if (x.getImpl() != nullptr) {
    //     IloInt size = x.getSize();
    //     cout << x.getSize() << endl;
    // }else{
    //     cout << "x is empty" << endl;
    // }

    objFunction(this->model,this->x);
    constr_DemandSatif(this->model,this->x);
    constr_pLocations(this->model,this->y);
    if(strcmp(typeProb,"CPMP") == 0 || strcmp(typeProb,"cPMP") == 0  ){constr_maxCapacity(this->model,this->x,this->y);}

}


void PMP::objFunction(IloModel model, BoolVarMatrix x){

    cout << "[INFO] Adding Objective Function "<< endl;
    
    IloEnv env = model.getEnv();
    IloExpr objExpr(env);
    for (IloInt i = 0; i < num_customers; i++)
        for (IloInt j = 0; j < num_facilities; j++){
            objExpr += instance->getWeightedDist(j,i) * x[i][j];
        }
    model.add(IloMinimize(env, objExpr));
    objExpr.end();
}

void PMP::constr_DemandSatif(IloModel model, BoolVarMatrix x){

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


void  PMP::constr_maxCapacity(IloModel model, BoolVarMatrix x, IloBoolVarArray y){

    cout << "[INFO] Adding Max Capacity Constraints "<< endl;

    IloEnv env = model.getEnv();
    for (IloInt j = 0; j < num_facilities; j++){
        IloExpr expr(env);
        for (IloInt i = 0; i < num_customers; i++)
            expr += IloInt(instance->getCustWeight(i)) * x[i][j];
        model.add(expr <= IloInt(instance->getLocCapacity(j)) * y[j]);
        expr.end();
    }

}


void PMP::printSolution(IloCplex& cplex, BoolVarMatrix x, IloBoolVarArray y){
    
        cout << "Solution status = " << cplex.getStatus()   << endl;
        cout << "Solution value  = " << cplex.getObjValue() << endl;
        cout << "Time: " << timePMP << endl;
    
        // for (IloInt i = 0; i < num_customers; i++){
        //     for (IloInt j = 0; j < num_facilities; j++){
        //         if (cplex.getValue(x[i][j]) > 0.5)
        //             cout << "x[" << i << "][" << j << "] = " << cplex.getValue(x[i][j]) << endl;
        //     }
        // }
    
        // for (IloInt j = 0; j < num_facilities; j++){
        //     if (cplex.getValue(y[j]) > 0.5)
        //         cout << "y[" << j << "] = " << cplex.getValue(y[j]) << endl;
        // }


        cout << "Time: " << cplex.getTime() << endl;
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