#include "PMP.hpp"




PMP::PMP(const shared_ptr<Instance>& instance):instance(instance)
{
    this->instance = instance;
    this->instance->show();

}
PMP::~PMP()
{
    // this->instance.reset();
    this->env.end();
}














void PMP::exportILP(IloCplex& cplex)
{
    cplex.exportModel("model.lp");
}

void TFP_SCP_SIMP::solveILP(){
    
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