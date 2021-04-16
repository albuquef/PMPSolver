#include <set>
#include "parsing.hpp"

using namespace std;

int main() {
    string filename = "./provided/toulon/toulon_destination.txt";

    auto locations = get_vector(filename);

    // Extract unique locations
    set<uint_t> unique_locations;
    for (auto l:locations) {
        unique_locations.insert(l);
    }

    for (auto l:unique_locations) {
        cout << l << endl;
    }
    auto max = *unique_locations.rbegin();
    cout << max << endl;





    return 0;
}
