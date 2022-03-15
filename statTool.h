
#ifndef STATTOOL_H
#define STATTOOL_H

#include "demogRegionData.h"
#include "psRegionData.h"

#include "visitor.h"
#include "stats.h"

/* wrapper for some useful functions to report necessary data */
class statTool
{

public:
    /* call visitor pattern to create state data */
    static void createStateData(std::vector<unique_ptr<regionData>> &theData, Visitor &theStates);

    /* call visitor pattern to create county data */
    static void createCountyData(std::vector<unique_ptr<regionData>> &theData, Visitor &theCounties);

    /* call visitor pattern to create aggregate data using a specific criteria */
    static void createKeyData(std::vector<unique_ptr<regionData>> &theData, Visitor &theKeyed);

    /* helper functions to fill in arrays based on funciton pointers  - on mix*/
    static void gatherCountStatsDGPS(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                                     std::string p1, std::string p2);

    /* helper functions to fill in arrays based on funciton pointers  - on police hsooting only*/
    static void gatherCountStatsPSPS(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                                     std::string p1, std::string p2);

    static void gatherStatsPDGCPS(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                            std::string prop1, std::string pop1, std::string prop2);

    /* helper functions to fill in arrays based on funciton pointers  - on demog percentages*/
    static void gatherPerStats(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                               std::string prop1, std::string pop1, std::string prop2, std::string pop2);

    /* percents and counts on demog */
    static int gatherBothStats(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                               vector<double> &XCount, vector<double> &Ycount,
                               std::string prop1, std::string pop1, std::string prop2, std::string pop2,
                               std::string prop3, std::string prop4);

    /* compute statistics for demographic data for a given region expects,
    the region and function pointers for the methods to fill in - mix ps and demog */
    static void computeStatsMixRegionData(visitorCombine *theRegions,
                                          std::string p1, std::string p2);

    static void computeStatsMixRegionDataPC(visitorCombine *theRegions,
                                      std::string prop1, std::string pop1, std::string prop2);

    /* compute statistics for demographic data for a given region expects,
    the region and function pointers for the methods to fill in - two demog fields */
    static void computeStatsDemogRegionData(visitorCombine *theRegions,
                                            std::string prop1, std::string pop1, std::string prop2, std::string pop2,
                                            std::string prop3, std::string prop4);

    /* compute statistics for demographic data for a given region expects,
    the region and function pointers for the methods to fill in - two police shooting fields */
    static void computeStatsPSData(visitorCombine *theRegions,
                                   std::string p1, std::string p2);
};
#endif
