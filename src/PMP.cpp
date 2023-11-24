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
    this->instance->print();
    this->p = this->instance->get_p();
    this->num_facilities = this->instance->getLocations().size();
    this->num_customers = this->instance->getCustomers().size();

    cout << "value of p: " << this->p << endl;
    cout << "Number of facilities: " << num_facilities << endl;
    cout << "Number of customers: " << num_customers << endl;
    cout << "Type of problem: " << typeProb << endl;
    cout << "Binary Model: " << is_BinModel << endl;


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

        // exportILP(cplex);

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





    // model.add(y[355] == 1);
    // model.add(y[529] == 1);
    // model.add(y[580] == 1);
    // model.add(y[268] == 1);
    // model.add(y[622] == 1);
    

}


// void PMP::objFunction(IloModel model, BoolVarMatrix x){
template <typename VarType>
void PMP::objFunction(IloModel model, VarType x){   

    cout << "[INFO] Adding Objective Function "<< endl;
    
    IloEnv env = model.getEnv();
    IloExpr objExpr(env);
    for (IloInt i = 0; i < num_customers; i++)
        for (IloInt j = 0; j < num_facilities; j++){
            if(strcmp(typeProb,"PMP") == 0 || strcmp(typeProb,"pmp") == 0  ){objExpr += instance->getRealDist(j,i) * x[i][j];}
            else{objExpr += instance->getWeightedDist(j+1,i+1) * x[i][j];}
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


// void  PMP::constr_maxCapacity(IloModel model, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType>
void  PMP::constr_maxCapacity(IloModel model, VarType x, IloBoolVarArray y){


    cout << "[INFO] Adding Max Capacity Constraints "<< endl;

    IloEnv env = model.getEnv();
    for (IloInt j = 0; j < num_facilities; j++){
        IloExpr expr(env);
        for (IloInt i = 0; i < num_customers; i++)
            expr += IloNum(instance->getCustWeight(i+1)) * x[i][j];
        model.add(expr <= IloNum(instance->getLocCapacity(j+1)) * y[j]);
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
    
        for (IloInt i = 0; i < num_customers; i++){
            for (IloInt j = 0; j < num_facilities; j++){
                if (cplex.getValue(x[i][j]) > 0.001)
                    cout << "x[" << i+1 << "][" << j+1 << "] = " << cplex.getValue(x[i][j]) << endl;
            }
        }
    
        for (IloInt j = 0; j < num_facilities; j++){
            if (cplex.getValue(y[j]) > 0.5)
                cout << "y[" << j+1 << "] = " << cplex.getValue(y[j]) << endl;
        }


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