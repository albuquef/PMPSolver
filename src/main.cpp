#include <set>
#include <iostream>
#include <unistd.h>
#include "instance.hpp"

using namespace std;

int main() {
    string loc_filename = "./provided/toulon/toulon_destination.txt";
    string cust_filename = "./provided/toulon/toulon_origin.txt";
    string dist_filename = "./provided/toulon/toulon_orig_x_tps.txt";

    Instance instance(loc_filename, cust_filename, dist_filename, 5);

    Instance instance1 = instance;

    cout << instance.getDist(690, 1) << endl;


    return 0;
}
