#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "stats.h"

using namespace std;

double RadToDeg(double rad) { return (180.0 / M_PI) * rad; }

double calcXWeight(size_t numDistricts) { return (numDistricts / 2) + .5; }

double getDeclinationAngle(vector<double> demVoteShares) {
    vector<double> above50;
    vector<double> below50;

    sort(demVoteShares.begin(), demVoteShares.end());
    for (auto pct : demVoteShares) {
        (pct > 0.5) ? above50.push_back(pct - 0.5)
                    : below50.push_back(0.5 - pct);
    }
    // compute the X scaling factor for the best-fit lines
    double aboveMidPoint = calcXWeight(above50.size());
    double belowMidPoint = calcXWeight(below50.size());

    // calculate the angles from center of line to average of each half of
    // points
    double theta = atan2(stats::computeMean(below50), belowMidPoint);
    double gamma = atan2(stats::computeMean(above50), aboveMidPoint);

    // debug
    cout << "average dem win: " << stats::computeMean(above50) << endl;
    cout << "average rep win: " << stats::computeMean(below50) << endl;
    cout << "below 50(in degrees): " << 180.0 / M_PI * theta << endl;
    cout << "above 50(in degrees): " << 180.0 / M_PI * gamma << endl;

    // calculate the declination angle as 2/π * best fit angle of above -
    // best fit angle of below
    double declinationAngle = M_2_PI * (gamma - theta);
    return RadToDeg(declinationAngle);
}