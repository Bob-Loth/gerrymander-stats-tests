#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "gerryStats.h"
#include "stats.h"

using namespace std;

int main() {
    vector<double> washington = {.4033, .4442, .4722, .5094, .5678,
                                 .5728, .5928, .6246, .7124, .8604};
    cout << getDeclinationAngle(washington) << endl;
}