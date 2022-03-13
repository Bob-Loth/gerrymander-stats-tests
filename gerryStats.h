#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "stats.h"

using namespace std;

double getDeclinationAngle(vector<double> demVoteShares) {
    vector<double> above50;
    vector<double> below50;

    sort(demVoteShares.begin(), demVoteShares.end());
    for (auto pct : demVoteShares) {
        (pct > 0.5) ? above50.push_back(pct) : below50.push_back(pct);
    }
    // compute the scaling factor for the best-fit lines
    double aboveRatio =
        static_cast<double>(demVoteShares.size()) / above50.size();
    double belowRatio =
        static_cast<double>(demVoteShares.size()) / below50.size();

    // calculate the angles of the best fit lines
    double theta =
        atan2(0.5 - stats::computeMean(below50), (below50.size() / 2.0));
    double gamma =
        atan2((stats::computeMean(above50) - 0.5), (above50.size() / 2.0));

    // debug
    cout << "average dem win: " << stats::computeMean(above50) << endl;
    cout << "average rep win: " << stats::computeMean(below50) << endl;
    cout << "below 50(in degrees): " << 180.0 / M_PI * theta << endl;
    cout << "above 50(in degrees): " << 180.0 / M_PI * gamma << endl;

    // calculate the declination angle as 2/π * best fit angle of above -
    // best fit angle of below
    double declinationAngle = (M_2_PI) * (gamma - theta);
    return declinationAngle;
}