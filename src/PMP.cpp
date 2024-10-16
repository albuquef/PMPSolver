#include "PMP.hpp"

#include <mutex>


double get_cpu_time_pmp(){
    using clock = std::chrono::high_resolution_clock;
    auto now = clock::now();
    auto now_sec = std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch());
    return now_sec.count();
}

static std::string gap_outputFilename;
void PMP::set_gap_report_filename(string Method_name){
    gap_outputFilename="";
    if (!is_BinModel){gap_outputFilename += "gap_Cont";}
    else{ gap_outputFilename += "gap_Bin";}
    gap_outputFilename += "_service_" + instance->getTypeService() + "_p_" + to_string(p) + "_" + Method_name;
    if(instance->isCoverMode()) gap_outputFilename += "_cover_"+ typeSubarea;
    gap_outputFilename += ".csv";
}
struct CallbackParams {
    IloCplex cplex;
    IloNum startTime;
    IloNum lastPrintTime;
    IloNum lastBestBound;
    double gapThreshold;
    double timeThreshold;
    double lastGap = 1.0;
    std::chrono::steady_clock::time_point lastTime;
    bool useGapInfoCallback = false;
    bool useBreakCallbackLessThan1Percent = false;
    bool useBreakCallbackImprovementCheck = false;
    bool gapBelowOnePercent = false;
    double timeGapBelowOnePercent = 0.0;   // Flag to track if the gap is below 1%
    double timelimite_less_than_1perc;     // Represents duration in seconds

    // Constructor to initialize the parameters
    CallbackParams(const IloCplex& cplex_)
        : cplex(cplex_),
          startTime(cplex.getCplexTime()),
          lastPrintTime(cplex.getCplexTime()),
          lastBestBound(cplex_.getBestObjValue()),
          gapThreshold(0.01),   // 1% improvement
          timeThreshold(180),   // 300 seconds
          lastTime(std::chrono::steady_clock::now()),
          timelimite_less_than_1perc(120.0) {}  // 2 minutes (120 seconds)
};
// Combined callback function
ILOMIPINFOCALLBACK1(CombinedCallback, CallbackParams*, params) {
    try {
        double interval_time = 5.0; // seconds

        // GapInfoCallback functionality
        if (params->useGapInfoCallback && params->cplex.getCplexTime() - params->lastPrintTime >= interval_time) {
            std::ofstream outputTable;
            outputTable.open("./outputs/reports/" + gap_outputFilename, std::ios::app);

            if (!outputTable.is_open()) {
                std::cerr << "Error opening file: " << std::endl;
            } else {
                outputTable << std::fixed << std::setprecision(15) << getBestObjValue() << ";"; // bound obj value
                outputTable << std::fixed << std::setprecision(15) << getIncumbentObjValue() << ";"; // obj value
                outputTable << getNnodes() << ";"; // num nodes
                outputTable << getMIPRelativeGap() << ";"; // relative gap
                outputTable << params->cplex.getCplexTime() - params->startTime << ";"; // time cplex
                outputTable << "\n";
            }

            std::cout << "Time: " << params->cplex.getCplexTime() - params->startTime << " seconds" << std::endl;
            std::cout << "MIP Gap: " << getMIPRelativeGap() << std::endl;
            std::cout << "Nodes: " << getNnodes() << std::endl;
            std::cout << "Best Objective: " << std::fixed << std::setprecision(15) << getBestObjValue() << std::endl;
            std::cout << "Incumbent Obj:  " << std::fixed << std::setprecision(15) << getIncumbentObjValue() << std::endl;

            params->lastPrintTime = params->cplex.getCplexTime();
        }

        if (hasIncumbent()) {
            double currentGap = getMIPRelativeGap();
            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = currentTime - params->lastTime;
            double elapsedTime = elapsed.count(); // elapsed time in seconds

            // BreakCallback functionality for less than 1% gap
            if (params->useBreakCallbackLessThan1Percent) {
                if (currentGap < 0.01) {
                    if (!params->gapBelowOnePercent) {
                        // Start tracking the time when the gap first goes below 1%
                        params->gapBelowOnePercent = true;
                        params->timeGapBelowOnePercent = params->cplex.getCplexTime();
                    } else {
                        // Calculate the total elapsed time with the gap below 1%
                        double totalElapsedTime = params->cplex.getCplexTime() - params->timeGapBelowOnePercent;
                        if (totalElapsedTime >= params->timelimite_less_than_1perc) {
                            std::cout << "Stopping optimization: Gap is less than 1% for more than "
                                      << std::fixed << std::setprecision(2) << params->timelimite_less_than_1perc << " seconds." << std::endl;
                            abort();
                        }
                    }
                } else {
                    // Reset the gapBelowOnePercent flag if the gap goes above 1%
                    params->gapBelowOnePercent = false;
                }
            }

            // BreakCallback functionality for existing improvement check
            if (params->useBreakCallbackImprovementCheck) {
                if (elapsedTime >= params->timeThreshold) {
                    double gapImprovement = params->lastGap - currentGap;
                    if (gapImprovement / params->lastGap < params->gapThreshold) {
                        std::cout << "Stopping optimization: Relative gap did not improve by "
                                  << std::fixed << std::setprecision(2) << params->gapThreshold * 100 << "% in the last " << params->timeThreshold << " seconds." << std::endl;
                        abort();
                    }
                    params->lastGap = currentGap;
                    params->lastTime = currentTime;
                }
            }
        }


    } catch (const IloException &ex) {
        std::cerr << "Error in callback function: " << ex.getMessage() << std::endl;
        throw; // Rethrow the exception to terminate the program
    }
}
ILOMIPINFOCALLBACK4(GapInfoCallback, IloCplex, cplex, IloNum, startTime, IloNum, lastPrintTime, IloNum, lastBestBound) {

    try {

        // cout << "Callback called" << endl;
        double interval_time = 5.0; // seconds

        if (cplex.getCplexTime() - lastPrintTime >= interval_time) {

            ofstream outputTable;
            outputTable.open("./outputs/reports/"+gap_outputFilename,ios:: app);

            
            if (!outputTable.is_open()) {
                // cerr << "Error opening file: " << output_filename << endl;
                cerr << "Error opening file: " << endl;
                // return;
            }else{
                // outputTable << fixed << setprecision(15) << cplex.getObjValue() << ";"; // obj value
                outputTable << fixed << setprecision(15) << getBestObjValue() << ";"; // bound obj value
                outputTable << fixed << setprecision(15) << getIncumbentObjValue() << ";"; // obj value
                outputTable << getNnodes() << ";"; // num nodes
                outputTable << getMIPRelativeGap() <<";"; // relative gap
                outputTable << cplex.getCplexTime() - startTime <<  ";"; // time cplex
                outputTable << "\n";
            }
            // outputTable.close();


            // print the y values
            // std::cout << "Printing p_loc values: " << std::endl;
            // for(IloInt j = 0; j < num_facilities_global; j++){
            //     auto loc = locations_global[j];
            //     // if (cplex.getValue(y_global[j]) > 0.5)
            //     if (getIncumbentValue(y[j]) > 0.5)
            //         cout << loc  << " ";
            //     cout << endl;
            // }
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

    VERBOSE = true;    

    // this->instance = instance;
    this->typeServ = typeServ;
    this->typeProb = typeProb;
    this->is_BinModel = is_BinModel;
    this->p = this->instance->get_p();
    this->num_facilities = this->instance->getLocations().size();
    this->num_customers = this->instance->getCustomers().size();

    if (VERBOSE) {
        cout << "------------------------------------------------------" << endl;
        cout << "[INFO] Start CPLEX Model" << endl;
        cout << "Problem type: " << typeProb << endl;
        cout << "Value of p: " << this->p << endl;
        cout << "Number of facilities: " << num_facilities << endl;
        cout << "Number of customers: " << num_customers << endl;
        if (instance->get_isWeightedObjFunc()) cout << "Weighted Objective Function: true" << endl;
        else cout << "Weighted Objective Function: false" << endl;
        // cout << "Type of problem: " << typeProb << endl;
        if (strcmp(typeProb,"CPMP") == 0 || strcmp(typeProb,"cPMP") == 0 || strcmp(typeProb,"GAP") == 0) cout << "Capacity Model: true" << endl;
        else cout << "Capacity Model: false" << endl;
        if (is_BinModel == true) cout << "Binary Model: true" << endl;
        else cout << "Binary Model: false" << endl;
        if (CoverModel) cout << "Cover Model: true" << endl;
        else cout << "Cover Model: false" << endl;
        cout << "------------------------------------------------------" << endl;
    }
}
PMP::~PMP()
{
    env.end();
}

void PMP::run(string Method_name){
    try{
        initILP();

        // Parameters CPLEX
        if (timeLimit != 0) cplex.setParam(IloCplex::Param::TimeLimit, timeLimit);
        if (useMIPStart) addMIPStartSolution();
        if (BestBound != 0) cplex.setParam(IloCplex::Param::MIP::Tolerances::LowerCutoff, BestBound);
        if (!displayCPLEX) cplex.setParam(IloCplex::Param::MIP::Display, 0);  // Disable console output

        // ADD FOR BIG INSTANCES

        // 1. Set Memory Emphasis (1 = memory emphasis)
        // cplex.setParam(IloCplex::Param::Emphasis::Memory, 1);

        // 2. Set maximum working memory to 128 GB (in MB)
        cplex.setParam(IloCplex::Param::WorkMem, 120000); // 120 GB = 120000 MB

        // 3. Control node file storage (3 = store nodes on disk only when necessary)
        cplex.setParam(IloCplex::Param::MIP::Strategy::File, 3);


        // 4. Stop if the optimality gap is less than 0.5%
        // cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0.005); // 0.5% gap
        
        
        // // 4. Set number of threads (optional, depending on your CPU core count)
        // int numThreads = <number_of_threads>;  // Replace with the number of available threads
        // cplex.setParam(IloCplex::Param::Threads, numThreads);

        // END ADD FOR BIG INSTANCES



        // Callbacks CPLEX
        CallbackParams params(cplex);
        // bool add_break_callback = true;
        if (timeLimit == 0) add_break_callback = false; // if time limit is not set, do not use break callback
        
        double gapThreshold = 0.01; // alpha% improvement, e.g., 1% improvement
        double timeThreshold = 180000; // T seconds, e.g., 300 seconds
        params.gapThreshold = gapThreshold;
        params.timeThreshold = timeThreshold;

        double time_limit_with_gap_less_than_1perc = 3600; // limit of time with gap less than 1%
        params.timelimite_less_than_1perc = time_limit_with_gap_less_than_1perc;


        if (add_generate_reports || add_break_callback) {
            params.useGapInfoCallback = add_generate_reports;
            set_gap_report_filename(Method_name);
            params.useBreakCallbackLessThan1Percent = add_break_callback;
            // params.useBreakCallbackImprovementCheck = add_break_callback;
            params.useBreakCallbackImprovementCheck = false;

            cplex.use(CombinedCallback(env, &params));

            if (add_generate_reports) {
                cout << "Gap Cplex Reports: " << gap_outputFilename << endl;
                cout << "[CALLBACK] Generating reports cplex callbacks..." << endl;
            }

            if (add_break_callback) {
                cout << "[CALLBACK] Using BreakCallback..." << endl;
                cout << fixed << setprecision(2);
                if(params.useBreakCallbackImprovementCheck) cout << "Time Limit with not improving " <<  params.gapThreshold * 100 << "% (gap): " << params.timeThreshold << " seconds" << endl;
                cout << "Time Limit with Gap less than 1%: " << time_limit_with_gap_less_than_1perc << " seconds" << endl;
            }
        } 


        // Solve CPLEX
        cplex.exportModel("./model.lp");
        if (priorityVoteLocations.size() > 0) {createPriorityListLocations(model,y);}
        solveILP();

        bool verb = false;
        if (cplex.getStatus() == IloAlgorithm::Optimal || cplex.getStatus() == IloAlgorithm::Feasible){
            isFeasible_Solver = true;
            if (verb){
                if(is_BinModel == true) {printSolution(cplex,x_bin,y);}
                else {printSolution(cplex,x_cont,y);}
            }
        }else
            cout << "Solution status = " << cplex.getStatus()   << endl;

    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\n[ERROR] run method PMP" << endl;
        return;
    }
}

void PMP::run_GAP(unordered_set<uint_t> p_locations){
    try{
        this->p_locations = p_locations;

        VERBOSE = false; 

        initILP();
        // Set the output to a non-verbose mode
        cplex.setParam(IloCplex::Param::MIP::Display, 0);
        cplex.setOut(env.getNullStream());  // Disable console output
        // cplex.setLogStream(fileStream);     // Redirect log to a file stream
        // exportILP(cplex);
        solveILP();

        bool verb = false;
        if (cplex.getStatus() == IloAlgorithm::Optimal || cplex.getStatus() == IloAlgorithm::Feasible){
          
            isFeasible_Solver = true;
            if (verb){
                if(is_BinModel == true) {printSolution(cplex,x_bin,y);}
                else {printSolution(cplex,x_cont,y);}
            }
        }else
            if (verb) cout << "Solution status = " << cplex.getStatus()   << endl;
    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\n[ERROR] run_gap method PMP" << endl;
        return;
    }
}

void PMP::initVars(){

    IloEnv env = model.getEnv();

    // alloc memory and add to model forvars y_j
    y = IloBoolVarArray(env, static_cast<IloInt>(num_facilities));
    for(IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
        char name[50];
        sprintf(name, "y(%ld)", j+1);
        this->y[j].setName(name);
        model.add(this->y[j]);
    }

    // alloc memory forvars x_ij and add to model
    if(is_BinModel == true){
        this->x_bin = BoolVarMatrix(env, static_cast<IloInt>(num_customers));
        for(IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
            this->x_bin[i] = IloBoolVarArray(env, static_cast<IloInt>(num_facilities));
        
        for(IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
            for(IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
                char name[50];
                sprintf(name, "x(%ld,%ld)", i+1, j+1);
                this->x_bin[i][j].setName(name);
                model.add(this->x_bin[i][j]);
            }
    }else{
        this->x_cont = NumVarMatrix(env, static_cast<IloInt>(num_customers));
        for(IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
            this->x_cont[i] = IloNumVarArray(env, static_cast<IloInt>(num_facilities),0.0,1.0, ILOFLOAT);

        for(IloInt i = 0; i < static_cast<IloInt>(num_customers); i++)
            for(IloInt j = 0; j < static_cast<IloInt>(num_facilities); j++){
                char name[50];
                sprintf(name, "x(%ld,%ld)", i+1, j+1);
                this->x_cont[i][j].setName(name);
                model.add(this->x_cont[i][j]);
            }
    }

}

void PMP::addMIPStartSolution(){

    cout << "[INFO] Adding MIP start solution" << endl;
    // add initial_solution to cplex
    auto sol = this->initial_solution;

    IloEnv env = model.getEnv();
    // IloBoolVarArray startVar_y(env);
    IloNumVarArray startVar_y(env);
    // IloBoolArray startVal_y(env);
    IloNumArray startVal_y(env);

    auto p_locations = sol.get_pLocations();
    auto assignments = sol.getAssignments();

    for(IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
        if (p_locations.find(loc) != p_locations.end()){
            startVar_y.add(y[j]);
            startVal_y.add(1);
        }else{
            startVar_y.add(y[j]);
            startVal_y.add(0);
        }
    }

    // cplex.addMIPStart(startVar_y, startVal_y);

    IloNumVarArray startVar_x(env);
    IloNumArray startVal_x(env);

    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]){ 
            auto loc = a.node;
            auto dem_used = a.usage;

            if (is_BinModel){
                startVar_x.add(x_bin[instance->getCustIndex(cust)][instance->getLocIndex(loc)]);
                startVal_x.add(dem_used);
            }else{
                startVar_x.add(x_cont[instance->getCustIndex(cust)][instance->getLocIndex(loc)]);
                startVal_x.add(dem_used / instance->getCustWeight(cust));
            }
        }
    }


    cplex.addMIPStart(startVar_y, startVal_y);
    cplex.addMIPStart(startVar_x, startVal_x);


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
    if(CoverModel) {constr_Cover(model,y);}
    if(CoverModel_n2) {constr_Cover_n2(model,y);}
    if (UpperBound != 0) {constr_UpperBound(model,x);}
    if (instance->get_ThresholdDist() > 0) {constr_MaxDistance(model,x);}
    if (instance->get_MaxLimitAssignments() > 0) {constr_MaxAssignments(model,x);}
    if (add_constr_maxNeighbors_from_solution) {constr_MaxNeighborsFromSolution(model,y);}
    if (cuts_type != "none" && strcmp(typeProb,"PMP") != 0 || strcmp(typeProb,"pmp") != 0) {cut_UBvarX_closestj(model,x,y);}
}

void PMP::setCutsType(string cuts_type){
    this->cuts_type = cuts_type;
}


// void PMP::objFunction(IloModel model, BoolVarMatrix x){
template <typename VarType>
void PMP::objFunction(IloModel model, VarType x){   

    if (VERBOSE){cout << "[INFO] Adding Objective Function "<< endl;}
    
    // bool is_weighted_obj_func = false;
    bool is_weighted_obj_func = instance->get_isWeightedObjFunc();
    
    if (VERBOSE){
        if (is_weighted_obj_func){cout << "SUM (wi * dij * xij)" << endl;}
        else{cout << "SUM (dij * xij)" << endl;}
    }
    
    IloEnv env = model.getEnv();
    IloExpr objExpr(env);
    for(IloInt i = 0; i < num_customers; i++)
        for(IloInt j = 0; j < num_facilities; j++){
            // if(strcmp(typeProb,"PMP") == 0 || strcmp(typeProb,"pmp") == 0  ){objExpr += instance->getRealDist(j+1,i+1) * x[i][j];}
            // else{objExpr += instance->getWeightedDist(j+1,i+1) * x[i][j];}
            auto loc = instance->getLocations()[j];
            auto cust = instance->getCustomers()[i];

            if (is_weighted_obj_func){objExpr += instance->getWeightedDist(loc,cust) * x[i][j];
            }else{objExpr += instance->getRealDist(loc,cust) * x[i][j];}

            // objExpr += instance->getWeightedDist(loc,cust) * x[i][j];
            // objExpr += instance->getRealDist(loc,cust) * x[i][j];
            // objExpr +=  x[i][j];
        }
    model.add(IloMinimize(env, objExpr));
    objExpr.end();
}

// void PMP::constr_DemandSatif(IloModel model, BoolVarMatrix x){
template <typename VarType>
void PMP::constr_DemandSatif(IloModel model, VarType x){

    if (VERBOSE){cout << "[INFO] Adding Demand Satisfied Constraints "<< endl;}

    IloEnv env = model.getEnv();
    for(IloInt i = 0; i < num_customers; i++){
        IloExpr expr(env);
        for(IloInt j = 0; j < num_facilities; j++){
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
    for(IloInt j = 0; j < num_facilities; j++)
        expr += y[j];
    model.add(expr == p);
    expr.end();

}

template <typename VarType>
void PMP::constr_UBpmp(IloModel model, VarType x, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding UB Constraints "<< endl;}

    for(IloInt i = 0; i < num_customers; i++)
        for(IloInt j = 0; j < num_facilities; j++)
            model.add(x[i][j] <= y[j]);

}


// void  PMP::constr_maxCapacity(IloModel model, BoolVarMatrix x, IloBoolVarArray y){
template <typename VarType>
void  PMP::constr_maxCapacity(IloModel model, VarType x, IloBoolVarArray y){


    if (VERBOSE){cout << "[INFO] Adding Max Capacity Constraints "<< endl;}

    IloEnv env = model.getEnv();
    for(IloInt j = 0; j < num_facilities; j++){
        IloExpr expr(env);
        auto loc = instance->getLocations()[j];
        for(IloInt i = 0; i < num_customers; i++){
            auto cust = instance->getCustomers()[i];
            expr += IloNum(instance->getCustWeight(cust)) * x[i][j];
        }
        model.add(expr <= IloNum(instance->getLocCapacity(loc)) * y[j]);
        expr.end();
    }

}

void PMP::constr_GAP(IloModel model, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding GAP fixed p Constraints "<< endl;}

    auto locations = instance->getLocations();

    for(auto loc:locations){
        auto index_loc = instance->getLocIndex(loc);
        if (index_loc != 10000000 && (p_locations.find(loc) == p_locations.end())){
            model.add(y[index_loc] == 0);
            // cout << "index: " << index_loc << " loc: " << loc
        }else if (index_loc != 10000000 && (p_locations.find(loc) != p_locations.end())){
            model.add(y[index_loc] == 1);
            // cout << "index: " << index_loc << " loc: " << loc << endl;
        }
    }

    // IloEnv env = model.getEnv();
    // for(IloInt j = 0; j < num_facilities; j++){
    //     auto loc = instance->getLocations()[j];
    //     if (p_locations.find(loc) == p_locations.end()){
    //         model.add(y[j] == 0);
    //     }else{
    //         cout << "add pos: " << j << " loc: " << loc << endl;
    //         model.add(y[j] == 1);
    //     }
    // }

}


void PMP::constr_Cover(IloModel model, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding Cover Constraints "<< endl;}

    num_subareas = instance->getSubareasSet().size();   

    // print number of possible subareas to cover that exist at least one location
    // cout << "num_subareas: " << num_subareas << endl;



    if (p >= num_subareas){

        for (IloInt s = 0; s <= num_subareas; s++){
            IloExpr expr(env);
            // auto subarea = instance->getSubareasSet()[s];
            auto subarea = instance->getLocationsSubarea(s);
            bool y_exist = false;
            for(IloInt j = 0; j < num_facilities; j++){
                auto loc = instance->getLocations()[j];
                if (find(subarea.begin(), subarea.end(), loc) != subarea.end()){
                    expr += y[j];
                    y_exist = true;
                }
            }
            if(!subarea.empty() && y_exist)
                model.add(expr >= 1);
            // if(!subarea.empty() && y_exist){
            //     model.add(expr >= 1);
            //     cout << "subarea: " << s << " size: " << subarea.size() << endl;
            // }
            expr.end();
        }

    }else{

        for (IloInt s = 0; s < num_subareas; s++){
            IloExpr expr(env);
            // auto subarea = instance->getSubareasSet()[s];
            auto subarea = instance->getLocationsSubarea(s);
            bool y_exist = false;
            for(IloInt j = 0; j < num_facilities; j++){
                auto loc = instance->getLocations()[j];
                if (find(subarea.begin(), subarea.end(), loc) != subarea.end()){
                    expr += y[j];
                    y_exist = true;
                }
            }
            if(!subarea.empty() && y_exist)
                model.add(expr <= 1);
            expr.end();
        }


    }
}

void PMP::constr_Cover_n2(IloModel model, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding Cover Constraints "<< endl;}

    num_subareas = instance->getSubareasSet_n2().size();   

    if (p >= num_subareas){

        for (IloInt s = 0; s <= num_subareas; s++){
            IloExpr expr(env);
            // auto subarea = instance->getSubareasSet()[s];
            auto subarea = instance->getLocationsSubarea_n2(s);
            bool y_exist = false;
            for(IloInt j = 0; j < num_facilities; j++){
                auto loc = instance->getLocations()[j];
                if (find(subarea.begin(), subarea.end(), loc) != subarea.end()){
                    expr += y[j];
                    y_exist = true;
                }
            }
            if(!subarea.empty() && y_exist)
                model.add(expr >= 1);
            // if(!subarea.empty() && y_exist){
            //     model.add(expr >= 1);
            //     cout << "subarea: " << s << " size: " << subarea.size() << endl;
            // }
            expr.end();
        }

    }else{

        for (IloInt s = 0; s < num_subareas; s++){
            IloExpr expr(env);
            // auto subarea = instance->getSubareasSet()[s];
            auto subarea = instance->getLocationsSubarea_n2(s);
            bool y_exist = false;
            for(IloInt j = 0; j < num_facilities; j++){
                auto loc = instance->getLocations()[j];
                if (find(subarea.begin(), subarea.end(), loc) != subarea.end()){
                    expr += y[j];
                    y_exist = true;
                }
            }
            if(!subarea.empty() && y_exist)
                model.add(expr <= 1);
            expr.end();
        }
    }
}


template <typename VarType>
void PMP::constr_UpperBound (IloModel model, VarType x){
    
    if (VERBOSE){cout << "[INFO] Adding Upper Bound Constraint "<< endl;}

    IloEnv env = model.getEnv();
    IloExpr objExpr(env);
    for(IloInt i = 0; i < num_customers; i++)
        for(IloInt j = 0; j < num_facilities; j++){
            // if(strcmp(typeProb,"PMP") == 0 || strcmp(typeProb,"pmp") == 0  ){objExpr += instance->getRealDist(j+1,i+1) * x[i][j];}
            // else{objExpr += instance->getWeightedDist(j+1,i+1) * x[i][j];}
            auto loc = instance->getLocations()[j];
            auto cust = instance->getCustomers()[i];
            objExpr += instance->getWeightedDist(loc,cust) * x[i][j];
        }
    model.add(objExpr <= UpperBound);
    objExpr.end();


}

template <typename VarType>
void PMP::constr_MaxDistance(IloModel model, VarType x){

    // bool is_weighted_obj_func = false;
    bool is_weighted_obj_func = instance->get_isWeightedObjFunc();

    if (VERBOSE){cout << "[INFO] Adding Max Distance Constraints "<< endl;}
    cout << "Threshold Distance: " << instance->get_ThresholdDist() << endl;

    for(IloInt i = 0; i < num_customers; i++)
        for(IloInt j = 0; j < num_facilities; j++){
            auto loc = instance->getLocations()[j];
            auto cust = instance->getCustomers()[i];
            
            // PACA CONSTRAINT IS FIXED UNWEIGHTED 7200
            // if (is_weighted_obj_func){
            //     if (instance->getWeightedDist(loc,cust) > instance->get_ThresholdDist() + 0.01)
            //         model.add(x[i][j] == 0);
            // }
            // if (!is_weighted_obj_func){
            if (instance->getRealDist(loc,cust) > instance->get_ThresholdDist())
                    model.add(x[i][j] == 0);
            else
                model.add(x[i][j] <= y[j]);
            // }

        }
}

template <typename VarType>
void PMP::constr_MaxAssignments(IloModel model, VarType x){

    if (VERBOSE){cout << "[INFO] Adding Max Number Assigments of Constraints "<< endl;}
    cout << "Max Limit of Assigments: " << instance->get_MaxLimitAssignments() << endl; 
   

    IloEnv env = model.getEnv();
    // count number of assignments for each facility
    for(IloInt j = 0; j < num_facilities; j++){
        IloExpr expr(env);
        for(IloInt i = 0; i < num_customers; i++){
            expr += x[i][j];
        }
        model.add(expr <= int(instance->get_MaxLimitAssignments()));
    }

}

void PMP::constr_MaxNeighborsFromSolution(IloModel model, IloBoolVarArray y){

    uint_t MaxNeighbors = MaxNeighbors_with_solution;

    if (VERBOSE){cout << "[INFO] Adding Max Neighbors Constraints from Solution"<< endl;
                cout << "Max Neighbors from solution: " << MaxNeighbors<< endl;}
    

    if (MaxNeighbors == 0) return;

    IloEnv env = model.getEnv();

    if (p_locations_from_solution.size() > 0){
        for(auto ploc:p_locations_from_solution){
            auto index_loc = instance->getLocIndex(ploc);
            if (index_loc != 10000000){
                IloExpr expr(env);
                expr += y[index_loc];
                // auto k_locs = this->instance->get_kClosestLocations(ploc, MaxNeighbors);
                auto k_locs = this->instance->get_kClosestLocations_notin_plocs(ploc, MaxNeighbors, p_locations_from_solution);
                for(auto neighbor:k_locs){
                    auto index_neighbor = instance->getLocIndex(neighbor);
                    if (index_neighbor != 10000000){
                        expr += y[index_neighbor];
                    }
                }
                model.add(expr >= 1);
            }
        }
    }

}

template <typename VarType> 
void PMP::constr_fixedAllocs_from_solution(IloModel model, IloBoolVarArray y, VarType x){

    if (VERBOSE){cout << "[INFO] Adding Fixed Allocations Constraints from Solution and fixed locations"<< endl;}

    IloEnv env = model.getEnv();

    // fix vars y from fixed_p_locations
    for(auto ploc:fixed_p_locations){
        auto index_loc = instance->getLocIndex(ploc);
        if (index_loc != 10000000){
            model.add(y[index_loc] == 1);
        }
    }

    // fix vars x from intial solution assignments
    auto assignments = initial_solution.getAssignments();
    for (auto cust:instance->getCustomers()) {
        for (auto a:assignments[cust]){ 
            auto loc = a.node;
            auto dem_used = a.usage;
            if (is_BinModel){
                model.add(x[instance->getCustIndex(cust)][instance->getLocIndex(loc)] == dem_used);
            }else{
                model.add(x[instance->getCustIndex(cust)][instance->getLocIndex(loc)] == dem_used / instance->getCustWeight(cust));
            }
        }
    }

}


template <typename VarType> 
void PMP::cut_UBvarX_closestj(IloModel model, VarType x, IloBoolVarArray y){

    if (VERBOSE){cout << "[INFO] Adding Cuts xij <= yj"<< endl;}

    // add xii < yi
    // for(IloInt i = 0; i < num_customers; i++){
    //     auto cust = instance->getCustomers()[i];
    //     auto loc = instance->getClosestLoc(cust);
    //     model.add(x[i][instance->getLocIndex(loc)] <= y[instance->getLocIndex(loc)]);
    // }

    if (cuts_type == "PairwiseCut_closestJ"){
        if(VERBOSE){cout << "...forall i only for the closest j != i"<< endl;}
        for (auto cust:instance->getCustomers()) {
            // auto loc = instance->getClosestLoc(cust);
            auto loc = instance->getClosestLoc_notloc(cust,cust);
            // cout << "cust: " << cust << " loc: " << loc << endl;
            model.add(x[instance->getCustIndex(cust)][instance->getLocIndex(loc)] <= y[instance->getLocIndex(loc)]);
        }
    }else if (cuts_type == "PairwiseCut_allJ"){
        if(VERBOSE){cout << "...forall i for all j"<< endl;}
        for(IloInt i = 0; i < num_customers; i++){
            for(IloInt j = 0; j < num_facilities; j++){
                model.add(x[i][j] <= y[j]);
            }
        }
    }else {
        cout << "[WARN] cuts type not found" << endl;
    }

}

template <typename VarType>  
void PMP::printSolution(IloCplex& cplex, VarType x, IloBoolVarArray y){
    
        cout << "Solution status = " << cplex.getStatus()   << endl;
        cout << "Solution value  = " << cplex.getObjValue() << endl;
        double objectiveValue = cplex.getObjValue();
        cout << "Objective Value: " << fixed << setprecision(15) << objectiveValue << endl;
        cout << "Num. of var x>0: " << x.getSize() << endl;
        cout << "Num. of var y>0: " << y.getSize() << endl;
        cout << "Time to solve: " << timeSolver << endl;

        bool verbose_vars = false;
        if (verbose_vars){
            for(IloInt j = 0; j < num_facilities; j++){
                auto loc = instance->getLocations()[j];
                for(IloInt i = 0; i < num_customers; i++){
                    auto cust = instance->getCustomers()[i];
                    if (cplex.getValue(x[i][j]) > 0.001)
                        cout << "x[" << cust << "][" << loc << "] = " << cplex.getValue(x[i][j]) << endl;
                }
            }
        
            for(IloInt j = 0; j < num_facilities; j++){
                auto loc = instance->getLocations()[j];
                if (cplex.getValue(y[j]) > 0.5)
                    cout << "y[" << loc << "] = " << cplex.getValue(y[j]) << endl;
            }
        }

        cout << "Time total: " << cplex.getTime() << endl;
}

void PMP::exportILP(IloCplex& cplex)
{
    cplex.exportModel("model.lp");
}

void PMP::solveILP(){
    double cpu0, cpu1;
    cpu0 = get_cpu_time_pmp(); 
    if (!cplex.solve()){
        isFeasible_Solver = false;
        // env.error() << "Failed to optimize LP." << endl;
        // cout << "Solution status = " << cplex.getStatus()   << endl;
        // throw(-1);
    }
    cpu1 = get_cpu_time_pmp();
    this->timeSolver = cpu1 - cpu0; 
}

Solution_cap PMP::getSolution_cap(){

    // cout << "[INFO] Getting solution capacitated" << endl;

    try{
        unordered_set<uint_t> p_locations;
        auto locations = instance->getLocations();

        for(IloInt j = 0; j < num_facilities; j++){
            auto loc = instance->getLocations()[j];
            if (cplex.getValue(y[j]) > 0.5)
                p_locations.insert(loc);
        }

        unordered_map<uint_t, dist_t> loc_usages; // p location -> usage from <0, capacity>
        unordered_map<uint_t, dist_t> cust_satisfactions; // customer -> satisfaction from <0, weight>
        unordered_map<uint_t, assignment> assignments; // customer -> assignment (p location, usage, weighted distance)

        for(auto p_loc:p_locations) loc_usages[p_loc] = 0;
        for(auto cust:instance->getCustomers()) {
            cust_satisfactions[cust] = 0;
            assignments[cust] = assignment{};
        }


        bool is_weighted_obj_func = instance->get_isWeightedObjFunc();
        dist_t objtest = 0.0;

        for(IloInt j = 0; j < num_facilities; j++){
            if (cplex.getValue(y[j]) > 0.5){

                auto loc = instance->getLocations()[j];
                for(IloInt i = 0; i < num_customers; i++){
                    auto cust = instance->getCustomers()[i];
                    
                    auto qtde_used = 0.0;
                    if (is_BinModel && cplex.getValue(x_bin[i][j]) > 0.5){
                        qtde_used = cplex.getValue(x_bin[i][j]);
                    }else if (!is_BinModel && cplex.getValue(x_cont[i][j]) > 0){
                        qtde_used = cplex.getValue(x_cont[i][j]);
                    }

                    if (qtde_used > 0.0001) {
                        auto dem_used = qtde_used*instance->getCustWeight(cust);
                        loc_usages[loc] += dem_used;
                        cust_satisfactions[cust] += dem_used;
                        assignments[cust].emplace_back(my_tuple{loc, dem_used, instance->getRealDist(loc, cust)});
                        
                        auto obj_increment =  instance->getRealDist(loc, cust) * qtde_used;
                        if (is_weighted_obj_func) obj_increment = dem_used * instance->getRealDist(loc, cust);
                        objtest += obj_increment;


                        // cout << "loc: " << loc << " cust: " << cust << " qtde_used: " << qtde_used << " dem_used: " << dem_used << " dist: " << instance->getRealDist(loc, cust) << " obj_increment: " << obj_increment << endl;
                    }
                    if(loc_usages[loc] >= instance->getLocCapacity(loc) + 0.01){ cerr << "[ERROR] usage > capacity" << endl;  exit(1);}
                    if(cust_satisfactions[cust] >= instance->getCustWeight(cust) + 0.01 ){ cerr << "[ERROR] satisfaction > weight" << endl; exit(1);}

                    
                }
            }


        }
        Solution_cap sol(instance, p_locations, loc_usages, cust_satisfactions, assignments);
        sol.setBestBound(static_cast<dist_t>(cplex.getBestObjValue()));
        
        return sol;

    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage()  << endl;
        cout << "\n[ERROR] get solution cap PMP" << endl;
        Solution_cap sol;
        sol.setFeasibility(false);
        return sol;
    }
}

Solution_std PMP::getSolution_std(){

    // cout << "[INFO] Getting solution standard" << endl;

    unordered_set<uint_t> p_locations;
    // auto p = instance->get_p();
    // auto locations = instance->getLocations();
    
    for(IloInt j = 0; j < num_facilities; j++){
            auto loc = instance->getLocations()[j];
            if (cplex.getValue(y[j]) > 0.5)
                p_locations.insert(loc);
        }

    Solution_std sol(instance, p_locations);

    return sol;
}

void PMP::saveVars(const std::string& filename,const string& Method){

    cout << "[INFO] Saving variables cplex:" << endl;

    string delimiter = "/";
    string directory;
    string rem_filename;

    // Find the last occurrence of the delimiter
    size_t pos = filename.find_last_of(delimiter);
    if (pos != std::string::npos) {
        // Extract the substring up to and including the last delimiter
        directory = filename.substr(0, pos + 1);
        rem_filename = filename.substr(pos + 1);
    } else {
        std::cerr << "[WARN] Delimiter not found in the filename string" << std::endl;
    }


    string output_filename = directory + "VarsValues_cplex/" + rem_filename + "_" + typeProb +
    "_Vars";
    if (is_BinModel){output_filename += "_Bin"; }

    output_filename += "_p_" + to_string(p) + "_"+ Method;
    
    if (CoverModel) output_filename += "_cover_" + typeSubarea;

    output_filename += ".txt";

    cout << output_filename  << endl;


    fstream file;
    streambuf *stream_buffer_cout = cout.rdbuf();
    // Open file if output_filename is not empty
    if (!output_filename.empty()) {
        file.open(output_filename, ios::out);
        streambuf *stream_buffer_file = file.rdbuf();
        cout.rdbuf(stream_buffer_file); // redirect cout to file
    }

    for(IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
        if (cplex.getValue(this->y[j]) > 0.5)
            cout << "y[" << loc << "] = " << cplex.getValue(this->y[j]) << endl;
    }

    if (!is_BinModel){
        for(IloInt j = 0; j < num_facilities; j++){
            auto loc = instance->getLocations()[j];
            for(IloInt i = 0; i < num_customers; i++){
                auto cust = instance->getCustomers()[i];
                if (cplex.getValue(x_cont[i][j]) > 0.001)
                    cout << "x[" << cust << "][" << loc << "] = " << cplex.getValue(x_cont[i][j]) << endl;
            }
        }
    }else{
        for(IloInt j = 0; j < num_facilities; j++){
        auto loc = instance->getLocations()[j];
        for(IloInt i = 0; i < num_customers; i++){
            auto cust = instance->getCustomers()[i];
            if (cplex.getValue(x_bin[i][j]) > 0.001)
                cout << "x[" << cust << "][" << loc << "] = " << cplex.getValue(x_bin[i][j]) << endl;
            }
        }
    }

    cout.rdbuf(stream_buffer_cout);
    file.close();

}

void PMP::setMIPStartSolution(Solution_cap solut){
    this->useMIPStart = true;
    this->initial_solution = solut;
}


void PMP::saveResults(const string& filename,const string& Method){

    cout << "[INFO] Saving results cplex: " << endl;

    string delimiter = "/";
    string directory;
    string rem_filename;

    // Find the last occurrence of the delimiter
    size_t pos = filename.find_last_of(delimiter);
    if (pos != std::string::npos) {
        // Extract the substring up to and including the last delimiter
        directory = filename.substr(0, pos + 1);
        rem_filename = filename.substr(pos + 1);
    } else {
        std::cerr << "[WARN] Delimiter not found in the filename string" << std::endl;
    }

    string output_filename = directory + "Results_cplex/" + rem_filename;
    if (!is_BinModel){output_filename += "_Cont_";}
    else{output_filename += "_Bin_";}
    output_filename += Method + ".csv";

    cout << output_filename << endl;

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
    // outputTable.close();

    // save results all cplex

    string output_filename_all = directory + "Results_cplex/results_all_cplex.csv";
    ofstream outputTable_all;
    outputTable_all.open(output_filename_all,ios:: app);

    if (!outputTable.is_open()) {
        cerr << "Error opening file: " << output_filename_all << endl;
        // return;
    }else{
        // add the date and hour of the execution
        time_t now = time(0);
        tm *ltm = localtime(&now);
        outputTable_all << 1900 + ltm->tm_year << "-" << 1 + ltm->tm_mon << "-" << ltm->tm_mday << ";";
        outputTable_all << typeProb << ";"; 
        if(is_BinModel == true)
            outputTable_all << "Bin" << ";";
        else
            outputTable_all << "Cont" << ";"; 
        outputTable_all << num_customers << ";";
        outputTable_all << num_facilities << ";";
        outputTable_all << p << ";";
        outputTable_all << Method << ";";
        if (instance->get_isWeightedObjFunc()) outputTable_all << "weighted_obj" << ";";
        else outputTable_all << "non-weighted_obj" << ";";
        if (CoverModel) outputTable_all << instance->getTypeSubarea() << ";";
        else outputTable_all << "non-cover_mode" << ";";
        if (CoverModel_n2) outputTable_all << instance->getTypeSubarea_n2() << ";";
        else outputTable_all << "non-cover_mode_n2" << ";";
        // outputTable_all << instance->get_isWeightedObjFunc() << ";";
        // outputTable_all << instance->isCoverMode() << ";";
        // outputTable_all << instance->isCoverMode_n2() << ";";
        outputTable_all << instance->getTypeService() << ";";
        outputTable_all << instance->getTypeSubarea() << ";"; 
        outputTable_all << cplex.getStatus() << ";"; // Status cplex
        // double objectiveValue = cplex.getObjValue();
        outputTable_all << fixed << setprecision(15) << cplex.getObjValue() << ";"; // obj value
        outputTable_all << cplex.getNnodes() << ";"; // num nodes
        outputTable_all << cplex.getMIPRelativeGap() <<";"; // relative gap
        outputTable_all << cplex.getTime() <<  ";"; // time cplex
        outputTable_all << this->timeSolver <<  ";"; // solver local time
        outputTable_all << "\n";
    }

    outputTable.close();
    outputTable_all.close();



}

bool PMP::getFeasibility_Solver(){
    return isFeasible_Solver;
}

void PMP::setGenerateReports(bool add_generate_reports){
    this->add_generate_reports = add_generate_reports;
}

void PMP::setAddBreakCallback(bool add_break_callback){
    this->add_break_callback = add_break_callback;
}

void PMP::setCoverModel(bool CoverModel, string typeSubarea){
    this->CoverModel = CoverModel;
    this->typeSubarea = typeSubarea;
}

void PMP::setCoverModel_n2(bool CoverModel_n2, string typeSubarea_n2){
    this->CoverModel_n2 = CoverModel_n2;
    this->typeSubarea_n2 = typeSubarea_n2;
}
// not wotking
void PMP::setSolution_cap(Solution_cap sol){
        
    cout << "[INFO] Setting CPLEX variables from solution" << endl;

    try {
        model = IloModel(env);
        initVars();

        if(is_BinModel == true)
            createModel(this->model,this->x_bin,this->y);
        else
            createModel(this->model,this->x_cont,this->y);  

        IloEnv env = model.getEnv();
        IloBoolVarArray startVar_y(env);
        IloBoolArray startVal_y(env);

        auto p_locations = sol.get_pLocations();
        auto sol_assignments = sol.getAssignments();

        // Set y variables based on the selected locations in the solution
        for(IloInt j = 0; j < num_facilities; j++) {
            auto loc = instance->getLocations()[j];
            if (p_locations.find(loc) != p_locations.end()) {
                // cplex.setValue(y[j], 1.0);
                startVar_y.add(y[j]);
                startVal_y.add(IloBool(1));
            } else {
                // cplex.setValue(y[j], 0.0);
                startVar_y.add(y[j]);
                startVal_y.add(IloBool(0));
            }        
        }

        std::vector<std::vector<IloNum>> X_matrix(num_customers, std::vector<IloNum>(num_facilities));
        for(auto& num_customers : X_matrix) {
            std::fill(num_customers.begin(), num_customers.end(), IloNum(0));
        }


        for(IloInt i = 0; i < num_customers; i++){
            auto cust = instance->getCustomers()[i];
            for(IloInt j = 0; j < num_facilities; j++){
                    auto loc_j = instance->getLocations()[j];
                for(auto a:sol_assignments[cust]) {
                    auto loc = a.node;
                    if (loc == loc_j) {
                        X_matrix[i][j] = IloNum(1);
                    }    
                }
            }
        }


        NumVarMatrix startVar_x(env);
        IloArray<IloNumArray> startVal_x(env);

        for(IloInt i = 0; i < num_customers; i++){
            IloNumArray startVal_x_i(env);
            for(IloInt j = 0; j < num_facilities; j++){
                startVal_x_i.add(X_matrix[i][j]);
                // startVar_x.add(x_cont[i][j]);
            }
            startVal_x.add(startVal_x_i);
            startVar_x.add(x_cont[i]);
        }


                // Now, you can proceed to solve the optimization problem using CPLEX
        try {

            this->cplex = IloCplex(this->model);

            // cplex.addMIPStart(startVar_y, startVal_y);
            // cplex.addMIPStart(startVar_x, startVal_x);
            // Solve the CPLEX model
            cplex.solve();
            
            // Check the solution status
            if (cplex.getStatus() == IloAlgorithm::Optimal || cplex.getStatus() == IloAlgorithm::Feasible){
                cout << "CPLEX found an feasible/optimal solution." << endl;
                // You can access the objective value and other information here
            } else {
                cout << "CPLEX did not find an optimal solution." << endl;
                // Handle other solution statuses if needed
            }
        } catch (IloException& e) {
            cerr << "ERROR: " << e.getMessage() << endl;
            cout << "\nError solving the CPLEX model" << endl;
        }


    } catch (IloException& e) {
        cerr << "ERROR: " << e.getMessage() << endl;
        cout << "\nError setting CPLEX variables from solution" << endl;
    }



}

void PMP::setUpperBound(double UB){
    this->UpperBound = UB;
}

void PMP::setTimeLimit(double CLOCK_LIMIT){
    // this->timeLimit =  static_cast<int>(ceil(CLOCK_LIMIT));
    this->timeLimit =  CLOCK_LIMIT;
}


void PMP::set_pLocations_from_solution(unordered_set<uint_t> p_locations){
    this->add_constr_maxNeighbors_from_solution = true;
    this->p_locations_from_solution = p_locations;
}
void PMP::set_MaxNeighbors_from_solution(uint_t MaxNeighbors){
    this->MaxNeighbors_with_solution = MaxNeighbors;
}

void PMP::set_Fixed_pLocations_from_solution(unordered_set<uint_t> fixed_p_locations){
    this->add_constr_FixedAllocs_from_solution = true;
    this->fixed_p_locations = fixed_p_locations;
}

void PMP::setBestBound(dist_t bestBound){
    this->BestBound = bestBound;
}

void PMP::setDisplayCPLEX(bool displayCPLEX){
    this->displayCPLEX = displayCPLEX;
}

void PMP::set_PriorityListLocations(vector<uint_t> priorityVoteLocations, string priorityStrategy){
    this->priorityVoteLocations = priorityVoteLocations;
    this->priorityStrategy = priorityStrategy;
}

void PMP::createPriorityListLocations(IloModel model, IloBoolVarArray y){


    if (VERBOSE){cout << "[INFO] Adding Priority List Locations "<< endl;}
    cout << "Priority List Locations: " << priorityVoteLocations.size() << endl;
    cout << "Priority Strategy: " << priorityStrategy << endl;
    
    if (priorityVoteLocations.size() == 0){
        cout << "[WARN] Priority List Locations is empty" << endl;
        return;
    }
    
    // order locations based on capacity
    vector<pair<uint_t, dist_t>> loc_capacities;
    if (priorityStrategy == "capacity_based") {
        for(auto loc: instance->getLocations()) {
            loc_capacities.push_back(make_pair(loc, instance->getLocCapacity(loc)));
        }
        // Sort by descending capacity
        sort(loc_capacities.begin(), loc_capacities.end(), [](const pair<uint_t, dist_t>& a, const pair<uint_t, dist_t>& b) {
            return a.second > b.second;
        });
    }

    IloEnv env = model.getEnv();
    try {
        // Initialize the priority array
        IloNumArray priorities(env, num_facilities);

        // Assign priorities based on the priorityVoteLocations list
        for (IloInt j = 0; j < num_facilities; ++j) {
            auto loc = instance->getLocations()[j];


           if (priorityStrategy == "index_based") {
                // Index-based: Use the index of the location in the priority list
                auto it = std::find(priorityVoteLocations.begin(), priorityVoteLocations.end(), loc);
                if (it != priorityVoteLocations.end()) {
                    auto index = std::distance(priorityVoteLocations.begin(), it);
                    priorities[j] = index;  // Set priority based on index
                } else {
                    priorities[j] = priorityVoteLocations.size() + 1;  // Low priority if not found
                }
            } 
            else if (priorityStrategy == "presence_based") {
                // Presence-based: Assign high (1) or low (2) priority based on location presence in the list
                if (std::find(priorityVoteLocations.begin(), priorityVoteLocations.end(), loc) != priorityVoteLocations.end()) {
                    priorities[j] = 1;  // High priority
                } else {
                    priorities[j] = 2;  // Low priority
                }
            }
            else if (priorityStrategy == "capacity_based") {
                if (loc_capacities.size() != 0) {
                    // assign priorities based on the order of the locations
                    auto it = std::find_if(loc_capacities.begin(), loc_capacities.end(),
                                        [loc](const std::pair<uint_t, dist_t>& loc_capacity) {
                                            return loc_capacity.first == loc;
                                        });
                    if (it != loc_capacities.end()) {
                        auto index = std::distance(loc_capacities.begin(), it);
                        priorities[j] = index + 1;  // Set priority based on index
                    } else {
                        priorities[j] = loc_capacities.size() + 1;  // Low priority if not found
                    }
                } else {
                    cerr << "Capacity-based priority strategy requires location capacities" << endl;
                }
            }
            else {
                cerr << "Invalid priority strategy: " << priorityStrategy << endl;
            }

        }

        // Set priorities for the variable array `y`
        cplex.setPriorities(y, priorities);
    }
    catch (const IloException& e) {
        cerr << "Error in setting priorities: " << e.getMessage() << endl;
    }

    // // Clean up
    // priorities.end();
}

void PMP::setPriorityStrategy(string priorityStrategy){
    this->priorityStrategy = priorityStrategy;
}