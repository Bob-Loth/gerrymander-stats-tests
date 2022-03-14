#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include "gerryStats.h"
#include "stats.h"

using namespace std;

int main() {
    cout << setprecision(3);
    vector<double> washington = {.4033, .4442, .4722, .5094, .5678,
                                 .5728, .5928, .6246, .7124, .9604};
    cout << "\ntesting with washington's 10 districts" << endl;
    cout << getDeclinationAngle(washington) << endl;
    cout << "mean-medians for washington (close to 0 is symmetric, negative is "
            "more favorable than positive):"
         << endl;
    double mm = getMeanMedianScores(washington);
    cout << "mean-median (positive values indicate advantage to republicans): "
         << mm << endl;
    vector<double> testOddNum = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
                                 0.6, 0.7, 0.8, 0.9, 1.0};
    cout << "\ntesting with 11 values" << endl;
    cout << getDeclinationAngle(testOddNum) << endl;

    vector<double> testEvenNum1 = {0.3,  0.4, 0.4, 0.4, 0.4, 0.45,
                                   0.55, 0.6, 0.7, 0.8, 0.9, 1.0};
    cout << "\ntesting with 12 values" << endl;
    cout << getDeclinationAngle(testEvenNum1) << endl;
    cout << getMeanMedianScores(testEvenNum1) << endl;

    vector<double> testEvenNum2 = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};
    cout << "\ntesting with 6 values" << endl;
    cout << getDeclinationAngle(testEvenNum2) << endl;
}