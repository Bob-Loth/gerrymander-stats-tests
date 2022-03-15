#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <iomanip>
#include <math.h>
#include "demogRegionData.h"
#include "psRegionData.h"
#include "parse.h"

#include "visitorReport.h"
#include "visitorCombineState.h"
#include "visitorCombineCounty.h"
#include "stats.h"
#include "statTool.h"

static void writeToCSV(vector<double> &x, vector<double> y, string file)
{
    // Create an input filestream
    std::ofstream myFile(file);

    // Make sure the file is open
    if (!myFile.is_open())
    {
        throw std::runtime_error("Could not open file");
    }

    if (myFile.good())
    {
        for (int i = 0; i < x.size(); i++)
        {
            myFile << x.at(i) << ", " << y.at(i) << endl;
        }
    }
    // Close file
    myFile.close();
}

/* statTool:: wrapper for some useful functions to report necessary data */
/* call visitor pattern to create state data */
void statTool::createStateData(std::vector<unique_ptr<regionData>> &theData, Visitor &theStates)
{

    // use visitor pattern to be able to aggregate
    for (const auto &obj : theData)
    {
        obj->accept((visitorCombineState &)theStates);
    }
}

/* call visitor pattern to create county data */
void statTool::createCountyData(std::vector<unique_ptr<regionData>> &theData, Visitor &theCounties)
{

    // use visitor pattern to be able to aggregate
    for (const auto &obj : theData)
    {
        obj->accept((visitorCombineCounty &)theCounties);
    }
}

/* call visitor pattern to create key data */
void statTool::createKeyData(std::vector<unique_ptr<regionData>> &theData, Visitor &theKeyed)
{

    // use visitor pattern to be able to aggregate
    for (const auto &obj : theData)
    {
        obj->accept((visitorCombineCounty &)theKeyed);
    }
}

/* helper functions to fill in arrays based on properties  - on mix*/
void statTool::gatherCountStatsDGPS(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                                std::string p1, std::string p2)
{
    // for all demographic data
    for (auto entry : theAggregate->getRegionDataDemog())
    {
        // make sure there is valid police shooting data!
        shared_ptr<psRegionData> temp = theAggregate->getRegionDataPoliceData(entry.first);
        psRegionData *thePSData = temp.get();
        if (thePSData != NULL)
        {
            double xP = (double)((entry.second).get()->getPropertyCount(p1));
            double yP = (double)(thePSData->getPropertyCount(p2));
            if (!isnan(xP) && !isnan(yP))
            {
                YPer.push_back(yP);
                XPer.push_back(xP);
            }
        }
    }
}

/* helper functions to fill in arrays based on properties  - on mix*/
void statTool::gatherStatsPDGCPS(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                                std::string prop1, std::string pop1, std::string prop2)
{
    // for all demographic data
    for (auto entry : theAggregate->getRegionDataDemog())
    {
        // make sure there is valid police shooting data!
        shared_ptr<psRegionData> temp = theAggregate->getRegionDataPoliceData(entry.first);
        psRegionData *thePSData = temp.get();
        if (thePSData != NULL)
        {
            double xP = (double)((entry.second).get()->getPropertyPercentage(prop1, pop1));
            double yP = (double)(thePSData->getPropertyCount(prop2));
            if (!isnan(xP) && !isnan(yP))
            {
                YPer.push_back(yP);
                XPer.push_back(xP);
            }
        }
    }
}

/* helper functions to fill in arrays based on funciton pointers  - on police hsooting only*/
void statTool::gatherCountStatsPSPS(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                                std::string p1, std::string p2)
{
    // for all demographic data
    for (auto entry : theAggregate->getRegionDataPolice())
    {
        psRegionData *thePSData = entry.second.get();
        if (thePSData != NULL)
        {
            double xP = (thePSData->getPropertyCount(p1));
            double yP = (thePSData->getPropertyCount(p2));
            if (!isnan(xP) && !isnan(yP) && !(xP == 0 && yP == 0))
            {
                YPer.push_back(yP);
                XPer.push_back(xP);
            }
        }
    }
}

/* helper functions to fill in arrays based on funciton pointers  - on demog percentages*/
void statTool::gatherPerStats(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                              std::string prop1, std::string pop1, std::string prop2, std::string pop2)
{
    // for all demographic data
    for (auto entry : theAggregate->getRegionDataDemog())
    {
        double xP = ((entry.second).get()->getPropertyPercentage(prop1, pop1));
        double yP = ((entry.second).get()->getPropertyPercentage(prop2, pop2));
        if (!isnan(xP) && !isnan(yP))
        {
            YPer.push_back(yP);
            XPer.push_back(xP);
        }
    }
}

/* percents and counts on demog */
int statTool::gatherBothStats(visitorCombine *theAggregate, vector<double> &XPer, vector<double> &YPer,
                              vector<double> &XCount, vector<double> &Ycount,
                              std::string prop1, std::string pop1, std::string prop2, std::string pop2,
                              std::string prop3, std::string prop4)
{

    // first functions for percentages
    gatherPerStats(theAggregate, XPer, YPer, prop1, pop1, prop2, pop2);

    // now fill in the counts
    int totPop = 0;
    for (auto entry : theAggregate->getRegionDataDemog())
    {
        shared_ptr<demogRegionData> demogForC = entry.second;
        double xC = ((entry.second).get()->getPropertyCount(prop3));
        double yC = ((entry.second).get()->getPropertyCount(prop4));
        if (!isnan(xC) && !isnan(yC))
        {
            XCount.push_back(xC);
            Ycount.push_back(yC);
            totPop += (entry.second)->getPropertyCount("Population.2020 Population");
        }
    }
    return totPop;
}

/* compute statistics for demographic data for a given region expects,
  the region and function pointers for the methods to fill in - two demog fields */
void statTool::computeStatsDemogRegionData(visitorCombine *theRegions,
                                           std::string prop1, std::string pop1, std::string prop2, std::string pop2,
                                           std::string prop3, std::string prop4)
{
    // data for both percents and counts
    vector<double> dataXcount;
    vector<double> dataYcount;
    vector<double> dataXpercent;
    vector<double> dataYpercent;

    // fill in the data
    int totPop = gatherBothStats(theRegions, dataXpercent, dataYpercent,
                                 dataXcount, dataYcount, prop1, pop1, prop2, pop2, prop3, prop4);

    double mX = stats::computePopMean(dataXcount, totPop);
    double mY = stats::computePopMean(dataYcount, totPop);
    cout << "REGION demographic statistics:" << endl;
    cout << "stats mean X: " << mX << " size of vector: " << dataXcount.size() << endl;
    cout << "stats mean Y: " << mY << " size of vector: " << dataYcount.size() << endl;

    cout << "std dev mean X: " << stats::computeStdDevPop(dataXpercent, mX) << endl;
    cout << "std dev mean Y: " << stats::computeStdDevPop(dataYpercent, mY) << endl;
    // cout << "Pearson Coeff: "<<stats::computeCorCoeff(dataXpercent, dataYpercent)<< endl;
    cout << "Population Coeff: " << stats::computeCorCoeffPop(dataXpercent, dataYpercent, mX, mY) << endl;

    writeToCSV(dataXcount, dataYcount, "demo/BryceCampbellProgram4/DemogCounts.csv");
    writeToCSV(dataXpercent, dataYpercent, "demo/BryceCampbellProgram4/DemogPercents.csv");
}

/* compute statistics for demographic data for a given region expects,
the region and function pointers for the methods to fill in - mix ps and demog */
void statTool::computeStatsMixRegionData(visitorCombine *theRegions,
                                         std::string p1, std::string p2)
{

    vector<double> dataX;
    vector<double> dataY;

    gatherCountStatsDGPS(theRegions, dataX, dataY, p1, p2);

    double mX = stats::computeMean(dataX);
    double mY = stats::computeMean(dataY);
    cout << "REGION stats comparing demographic and police shooting data " << endl;
    cout << "stats mean X: " << mX << " size of vector: " << dataX.size() << endl;
    cout << "stats mean Y: " << mY << " size of vector: " << dataY.size() << endl;

    cout << "std dev mean X: " << stats::computeStdDevSample(dataX) << endl;
    cout << "std dev mean Y: " << stats::computeStdDevSample(dataY) << endl;
    cout << "Correlation Coeff (sample): " << stats::computeCorCoeffSample(dataX, dataY) << endl;

    writeToCSV(dataX, dataY, "demo/BryceCampbellProgram4/MixCounts.csv");
}

/* compute statistics for demographic data for a given region expects, using percents
the region and function pointers for the methods to fill in - mix ps and demog */
void statTool::computeStatsMixRegionDataPC(visitorCombine *theRegions,
                                        std::string prop1, std::string pop1, std::string prop2)
{

    vector<double> dataX;
    vector<double> dataY;

    gatherStatsPDGCPS(theRegions, dataX, dataY, prop1, pop1, prop2);

    double mX = stats::computeMean(dataX);
    double mY = stats::computeMean(dataY);
    cout << "REGION stats comparing demographic and police shooting data " << endl;
    cout << "stats mean X: " << mX << " size of vector: " << dataX.size() << endl;
    cout << "stats mean Y: " << mY << " size of vector: " << dataY.size() << endl;

    cout << "std dev mean X: " << stats::computeStdDevSample(dataX) << endl;
    cout << "std dev mean Y: " << stats::computeStdDevSample(dataY) << endl;
    cout << "Correlation Coeff (sample): " << stats::computeCorCoeffSample(dataX, dataY) << endl;

    writeToCSV(dataX, dataY, "demo/BryceCampbellProgram4/MixCounts.csv");
}

/* compute statistics for demographic data for a given region expects,
the region and function pointers for the methods to fill in - two police shooting fields */
void statTool::computeStatsPSData(visitorCombine *theRegions,
                                  std::string p1, std::string p2)
{

    vector<double> dataX;
    vector<double> dataY;

    gatherCountStatsPSPS(theRegions, dataX, dataY, p1, p2);

    double mX = stats::computeMean(dataX);
    double mY = stats::computeMean(dataY);
    cout << "REGION stats comparing police shooting data two variables " << endl;
    cout << "stats mean X: " << mX << " size of vector: " << dataX.size() << endl;
    cout << "stats mean Y: " << mY << " size of vector: " << dataY.size() << endl;

    cout << "std dev mean X: " << stats::computeStdDevSample(dataX) << endl;
    cout << "std dev mean Y: " << stats::computeStdDevSample(dataY) << endl;
    cout << "Correlation Coeff (sample): " << stats::computeCorCoeffSample(dataX, dataY) << endl;

    writeToCSV(dataX, dataY, "demo/BryceCampbellProgram4/PSCounts.csv");
}
