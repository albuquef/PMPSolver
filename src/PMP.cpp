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

static std::string gap_outputFilename;
ILOMIPINFOCALLBACK4(GapInfoCallback, IloCplex, cplex, IloNum, startTime, IloNum, lastPrintTime, IloNum, lastBestBound) {
    try {

        // cout << "Callback called" << endl;
        double interval_time = 5.0; // seconds

        if (cplex.getCplexTime() - lastPrintTime >= interval_time) {

            ofstream outputTable;
            outputTable.open("./reports/"+gap_outputFilename,ios:: app);

            
            if (!outputTable.is_open()) {
                // cerr << "Error opening file: " << output_filename << endl;
                cerr << "Error opening file: " << endl;
                // return;
            }else{
                // outputTable << fixed << setprecision(15) << cplex.getObjValue() << ";"; // obj value
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
            // std::cout << "Best Integer: " <<  fixed << setprecision(15) << cplex.getObjValue() << std::endl;
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

PMP::PMP(const shared_ptr<Instance>& instance,const char* typeProb, bool is_BinModel):instance(instance)
{

    VERBOSE = false;    

    // this->instance = instance;
    this->typeServ = typeServ;
    this->typeProb = typeProb;
    this->is_BinModel = is_BinModel;
    this->p = this->instance->get_p();
    this->num_facilities = this->instance->getLocations().size();
    this->num_customers = this->instance->getCustomers().size();

    if (VERBOSE) {
        cout << "Problem type: " << typeProb << endl;
        cout << "Value of p: " << this->p << endl;
        cout << "Number of facilities: " << num_facilities << endl;
        cout << "Number of customers: " << num_customers << endl;
        // cout << "Type of problem: " << typeProb << endl;
        if (strcmp(typeProb,"CPMP") == 0 || strcmp(typeProb,"cPMP") == 0 || strcmp(typeProb,"GAP") == 0)
            cout << "Capacity Model: true" << endl;
        else 
            cout << "Capacity Model: false" << endl;
        if (is_BinModel == true) 
            cout << "Binary Model: true" << endl;
        else 
            cout << "Binary Model: false" << endl;
    }
}
PMP::~PMP()
{
    env.end();
}

void PMP::run(){
    try{
        initILP();


        if (CLOCK_LIMIT_CPLEX != 0) cplex.setParam(IloCplex::TiLim, CLOCK_LIMIT_CPLEX);

        // cplex.setParam(IloCplex::TiLim, 60);
        // cplex.setParam(IloCplex::TiLim, CLOCK_LIMIT); // time limit CLOCK_LIMIT seconds
        // cplex.setParam(IloCplex::TreLim, 30000); // tree memory limit 30GB
        // cplex.setParam(IloCplex::Threads, 8); // use 8 threads

        // Set up the MIP callback
        IloNum startTime = cplex.getCplexTime();
        IloNum lastPrintTime = startTime;
        IloNum lastBestBound = cplex.getBestObjValue();
        // gap_outputFilename = "gap.csv";
        if (!is_BinModel){
            gap_outputFilename = "gap_Cont_service_" + instance->getTypeService() +
                "_p_" + to_string(p) +
                ".csv";
        }else{
            gap_outputFilename = "gap_Bin_service_" + instance->getTypeService() +
                "_p_" + to_string(p) +
                ".csv";
        }
        cplex.use(GapInfoCallback(env, cplex, startTime, lastPrintTime, lastBestBound));


        solveILP();

        bool verbose = true;
        if (verbose){
            if (cplex.getStatus() == IloAlgorithm::Optimal)
                if(is_BinModel == true) {printSolution(cplex,x_bin,y);}
                else {printSolution(cplex,x_cont,y);}
            else
                cout << "Solution status = " << cplex.getStatus()   << endl;
        }
        // cplex.end();
        // env.end();
    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\nError ilocplex" << endl;
        return;
    }
}

void PMP:: run_GAP(unordered_set<uint_t> p_locations){
    try{
        this->p_locations = p_locations;

        initILP();
        // Set the output to a non-verbose mode
        cplex.setParam(IloCplex::Param::MIP::Display, 0);
        cplex.setOut(env.getNullStream());  // Disable console output
        // cplex.setLogStream(fileStream);     // Redirect log to a file stream
        solveILP();

        if (VERBOSE){
            if (cplex.getStatus() == IloAlgorithm::Optimal)
                if(is_BinModel == true) {printSolution(cplex,x_bin,y);}
                else {printSolution(cplex,x_cont,y);}
            else
                cout << "Solution status = " << cplex.getStatus()   << endl;
        }
        // cplex.end();
        // env.end();
    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\nError ilocplex" << endl;
        return;
    }
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
    if(strcmp(typeProb,"GAP") == 0) {constr_GAP(model,y);}
    if(strcmp(typeProb,"GAP") != 0) {constr_pLocations(model,y);}
    if(strcmp(typeProb,"CPMP") == 0 || strcmp(typeProb,"cPMP") == 0 || strcmp(typeProb,"GAP") == 0){constr_maxCapacity(model,x,y);}
    if(strcmp(typeProb,"PMP") == 0 || strcmp(typeProb,"pmp") == 0  ){constr_UBpmp(model,x,y);}

}


// void PMP::objFunction(IloModel model, BoolVarMatrix x){
template <typename VarType>
void PMP::objFunction(IloModel model, VarType x){   

    if (VERBOSE){cout << "[INFO] Adding Objective Function "<< endl;}
    
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

    if (VERBOSE){cout << "[INFO] Adding Demand Satisfied Constraints "<< endl;}

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

    if (VERBOSE){cout << "[INFO] Adding p Locations Constraints "<< endl;}

    IloEnv env = model.getEnv();
    IloExpr expr(env);
    for (IloInt j = 0; j < num_facilities; j++)
        expr += y[j];
    model.add(expr == p);
    expr.end();

}

template <typename VarType>
void PMP::constr_UBpmp(IloModel model, VarType x, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding UB Constraints "<< endl;}

    IloEnv env = model.getEnv();
    for (IloInt i = 0; i < num_customers; i++)
        for (IloInt j = 0; j < num_facilities; j++)
            model.add(x[i][j] <= y[j]);

}


// void  PMP::constr_maxCapacity(IloModel model, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType>
void  PMP::constr_maxCapacity(IloModel model, VarType x, IloBoolVarArray y){


    if (VERBOSE){cout << "[INFO] Adding Max Capacity Constraints "<< endl;}

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

void PMP::constr_GAP(IloModel model, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding GAP fixed p Constraints "<< endl;}

    IloEnv env = model.getEnv();
    for (IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
        if (p_locations.find(loc) == p_locations.end()){
            model.add(y[j] == 0);
        }else{
            model.add(y[j] == 1);
        }
    }

}



// void PMP::printSolution(IloCplex& cplex, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType>  
void PMP::printSolution(IloCplex& cplex, VarType x, IloBoolVarArray y){
    
        cout << "Solution status = " << cplex.getStatus()   << endl;
        cout << "Solution value  = " << cplex.getObjValue() << endl;
        double objectiveValue = cplex.getObjValue();
        cout << "Objective Value: " << fixed << setprecision(15) << objectiveValue << endl;
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
    this->timeSolver = cpu1 - cpu0; 
}

Solution_cap PMP::getSolution_cap(){

    // cout << "[INFO] Getting solution capacitated" << endl;

    try{
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

        // cout << "p_loc = ";
        // for (auto p_loc:p_locations)
        //     cout << p_loc << ", ";
        // cout << endl;   

        dist_t objtest = 0;

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
                        
                            cerr << "ERROR: usage > capacity" << endl;    
                            exit(1);
                    }
                    if(cust_satisfactions[cust] >= instance->getCustWeight(cust) + 0.001 ){

                        cerr << "ERROR: satisfaction > weight" << endl;
                        exit(1);
                    }

                    
                }
            }

        // cout << "obj: " << objtest << endl;

        Solution_cap sol(instance, p_locations, loc_usages, cust_satisfactions, assignments, objtest);

        return sol;
    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\nError get solution cap" << endl;
        return Solution_cap();
    }
}

Solution_std PMP::getSolution_std(){

    // cout << "[INFO] Getting solution standard" << endl;

    unordered_set<uint_t> p_locations;
    auto p = instance->get_p();
    // auto locations = instance->getLocations();
    
    for (IloInt j = 0; j < num_facilities; j++){
            auto loc = instance->getLocations()[j];
            if (cplex.getValue(y[j]) > 0.5)
                p_locations.insert(loc);
        }



    Solution_std sol(instance, p_locations);

    return sol;
}

void PMP::saveVars(const std::string& filename,const string& Method){

    cout << "[INFO] Saving variables" << endl;

    fstream file;
    streambuf *stream_buffer_cout = cout.rdbuf();

    string output_filename = "TEST.txt";
    if (!is_BinModel){
        output_filename = filename + "_" + typeProb +
            "_Vars_Cont_p_" + to_string(p) + 
            "_" + Method +
            ".txt";
    }else{
        output_filename = filename + "_" + typeProb +
            "_Vars_Bin_p_" + to_string(p) + 
            "_" + Method +
            ".txt";
    }

    // Open file if output_filename is not empty
    if (!output_filename.empty()) {
        file.open(output_filename, ios::out);
        streambuf *stream_buffer_file = file.rdbuf();
        cout.rdbuf(stream_buffer_file); // redirect cout to file
    }

    for (IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
        if (cplex.getValue(this->y[j]) > 0.5)
            cout << "y[" << loc << "] = " << cplex.getValue(this->y[j]) << endl;
    }

    if (!is_BinModel){
        for (IloInt j = 0; j < num_facilities; j++){
            auto loc = instance->getLocations()[j];
            for (IloInt i = 0; i < num_customers; i++){
                auto cust = instance->getCustomers()[i];
                if (cplex.getValue(x_cont[i][j]) > 0.001)
                    cout << "x[" << cust << "][" << loc << "] = " << cplex.getValue(x_cont[i][j]) << endl;
            }
        }
    }else{
        for (IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
            for (IloInt i = 0; i < num_customers; i++){
                auto cust = instance->getCustomers()[i];
                if (cplex.getValue(x_bin[i][j]) > 0.001)
                    cout << "x[" << cust << "][" << loc << "] = " << cplex.getValue(x_bin[i][j]) << endl;
            }
        }
    }

    cout.rdbuf(stream_buffer_cout);
    file.close();

}

void PMP::saveResults(const string& filename,const string& Method){

    cout << "[INFO] Saving results" << endl;

    string output_filename = "TEST.txt";
    if (!is_BinModel){
        output_filename = filename +
            "_Cont_" + Method +
            ".csv";
    }else{
        output_filename = filename +
            "_Bin_" + Method +
            ".csv";
    }

    ofstream outputTable;
    outputTable.open(output_filename,ios:: app);

    if (!outputTable.is_open()) {
        cerr << "Error opening file: " << output_filename << endl;
        // return;
    }else{
        outputTable << typeProb << ";"; 
        if(is_BinModel == true)
            outputTable << "Bin" << ";";
        else
            outputTable << "Cont" << ";"; 
        outputTable << num_customers << ";";
        outputTable << num_facilities << ";";
        outputTable << p << ";";
        outputTable << Method << ";";
        outputTable << cplex.getStatus() << ";"; // Status cplex
        // double objectiveValue = cplex.getObjValue();
        outputTable << fixed << setprecision(15) << cplex.getObjValue() << ";"; // obj value
        outputTable << cplex.getNnodes() << ";"; // num nodes
        outputTable << cplex.getMIPRelativeGap() <<";"; // relative gap
        outputTable << cplex.getTime() <<  ";"; // time cplex
        outputTable << this->timeSolver <<  ";"; // solver local time
        outputTable << "\n";
    }
    outputTable.close();
}


