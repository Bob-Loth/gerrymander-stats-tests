#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "stats.h"

double RadToDeg(double rad) { return (180.0 / M_PI) * rad; }

double calcXWeight(size_t numDistricts) { return (numDistricts / 2) + .5; }

// returns a vector with position 0 = republican mean-median, position 1 =
// democratic mean-median. the mean-median is a measure of the difference
// between a party's median vote share and its mean vote share. Higher
// differences, especially if the differences are largely only affecting one
// party in a state, could indicate a partisan gerrymander.
double getMeanMedianScores(std::vector<double> demVoteShares) {
    std::vector<double> demDistricts;
    std::vector<double> repDistricts;
    sort(demVoteShares.begin(), demVoteShares.end());
    // split into the percentage of the respective party's votes, rather than by
    // demVoteShares.
    for (auto pct : demVoteShares) {
        (pct > 0.5) ? demDistricts.push_back(pct)
                    : repDistricts.push_back(1 - pct);
    }
    double demMeanMedian =
        stats::computeMedian(demDistricts) - stats::computeMean(demDistricts);
    double repMeanMedian =
        stats::computeMedian(repDistricts) - stats::computeMean(repDistricts);

    return demMeanMedian - repMeanMedian;
}

// gets the two angles from a point on the 50% line inbetween democratic and
// republican districts, to the point representing each group's center of
// mass, and returns a measure of the difference between these two values.
// Close to 0 is better. Positive values may indicate gerrymander in favor
// of republicans, negative values may indicate gerrymander in favor of
// democrats.
double getDeclinationAngle(std::vector<double> demVoteShares) {
    // these are the absolute-value differences from 0.5, to make the math
    // easier
    std::vector<double> demDiff;
    std::vector<double> repDiff;
    // split into the two groups, and store as differences from 50%.
    sort(demVoteShares.begin(), demVoteShares.end());
    for (auto pct : demVoteShares) {
        (pct > 0.5) ? demDiff.push_back(pct - 0.5)
                    : repDiff.push_back(0.5 - pct);
    }
    // compute the X scaling factor for the best-fit lines
    double demCenter = calcXWeight(demDiff.size());
    double repCenter = calcXWeight(repDiff.size());

    // calculate the angles from center of line to average of each half of
    // points
    double repAngle = atan2(stats::computeMean(repDiff), repCenter);
    double demAngle = atan2(stats::computeMean(demDiff), demCenter);

    // calculate the declination angle as 2/π * best fit angle of above -
    // best fit angle of below
    double declinationAngle = M_2_PI * (demAngle - repAngle);
    return RadToDeg(declinationAngle);
}