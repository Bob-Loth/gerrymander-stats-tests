#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>

#include "stats.h"

double RadToDeg(double rad) { return (180.0 / M_PI) * rad; }

double calcXWeight(size_t numDistricts) { return (numDistricts / 2) + .5; }

// computes the efficiency gap. Gap scaling is based on population, due to lack
// of voting% data for our demVoteShares data. Positive values indicate
// advantage for Republicans by convention.
double getEfficiencyGap(std::vector<int> demVoteCounts,
                        std::vector<int> repVoteCounts) {
    int totalVotesCast = 0;
    totalVotesCast =
        std::accumulate(demVoteCounts.begin(), demVoteCounts.end(), 0) +
        std::accumulate(repVoteCounts.begin(), repVoteCounts.end(), 0);
    // this has an error of +-2%, as allowed by redistricting law

    int demWasted = 0;
    int repWasted = 0;
    // calculate wasted votes across districts where Democrats won
    for (int i = 0; i < demVoteCounts.size(); i++) {
        if (demVoteCounts[i] > repVoteCounts[i]) {
            repWasted += repVoteCounts[i];
            demWasted += demVoteCounts[i] - repVoteCounts[i];
        } else {
            demWasted += demVoteCounts[i];
            repWasted += repVoteCounts[i] - repVoteCounts[i];
        }
    }
    // this is based on population, because we don't have data on voting
    // percentage
    double efficiencyGap =
        static_cast<double>(demWasted - repWasted) / totalVotesCast;
    return efficiencyGap;
}

double getPartisanBias(std::vector<int> demVoteCounts,
                       std::vector<int> repVoteCounts) {
    int statewideDemVoteCount =
        std::accumulate(demVoteCounts.begin(), demVoteCounts.end(), 0);
    int statewideRepVoteCount =
        std::accumulate(repVoteCounts.begin(), repVoteCounts.end(), 0);
    int statewideTotalVoteCount = statewideDemVoteCount + statewideRepVoteCount;

    double statewideDemVoteShare =
        static_cast<double>(statewideDemVoteCount) / statewideTotalVoteCount;
    double statewideRepVoteShare =
        static_cast<double>(statewideRepVoteCount) / statewideTotalVoteCount;
    // the ratios to multiply each district's vote totals by to simulate a tied
    // statewide election
    double demRatio = 0.5 / statewideDemVoteShare;
    double repRatio = 0.5 / statewideRepVoteShare;

    std::vector<int> adjustedDemVoteCounts;
    std::vector<int> adjustedRepVoteCounts;
    // adjust each district's vote counts evenly to reflect this hypothetical
    // tied election
    std::transform(demVoteCounts.begin(), demVoteCounts.end(),
                   back_inserter(adjustedDemVoteCounts),
                   [demRatio](int val) { return val * demRatio; });
    std::transform(repVoteCounts.begin(), repVoteCounts.end(),
                   back_inserter(adjustedRepVoteCounts),
                   [repRatio](int val) { return val * repRatio; });
    // determine the new winners of districts based on this information
    int adjustedRepSeatsWon = 0;
    for (int i = 0; i < demVoteCounts.size(); i++) {
        if (adjustedRepVoteCounts[i] >= adjustedDemVoteCounts[i]) {
            adjustedRepSeatsWon++;
        }
    }
    double percentageRepSeatsWon =
        static_cast<double>(adjustedRepSeatsWon) / demVoteCounts.size();
    return percentageRepSeatsWon - 0.5;
}

// returns a double indicating (demMedian - demMean) - (repMedian - repMean)
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