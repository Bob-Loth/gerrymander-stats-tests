#ifndef VCOMBOC_H
#define VCOMBOC_H

#include "psRegionData.h"
#include "demogRegionData.h"
#include "visitorCombine.h"
#include "parse.h"
#include <memory>
#include <fstream>
#include <cassert>
#include <string>

class visitorCombineCounty : public visitorCombine
{
public:

    int noMatch;

    visitorCombineCounty() : noMatch(0)
    {
    }

    // store demographic data by county name
    void visit(demogRegionData *obj)
    {
        std::string countyState = obj->getCounty() + obj->getState();

        if (allComboDemogData.find(countyState) != allComboDemogData.end())
        {
            allComboDemogData[countyState]->aggregate(obj);
        }
        else
        {
            allComboDemogData[countyState] = std::make_shared<demogRegionData>(*obj);
        }
    }

    // aggregate police shooting data by county
    void visit(psRegionData *obj)
    {
        if(obj->getCounty() == "UNDEF") {
            noMatch++;
        }
        std::string countyState = obj->getCounty() + obj->getState();
        if (allComboPoliceData.find(countyState) != allComboPoliceData.end())
        {
            allComboPoliceData[countyState]->aggregate(obj);
        }
        else
        {
            allComboPoliceData[countyState] = std::make_shared<psRegionData>(*obj);
        }
    }

    // aggregate police shooting data by county
    void visit(districtRegionData *obj)
    {
        std::string districtState = obj->getDistrictNum() + obj->getState();
        if (allComboDistrictData.find(districtState) != allComboDistrictData.end())
        {
            allComboDistrictData[districtState]->aggregate(obj);
        }
        else
        {
            allComboDistrictData[districtState] = std::make_shared<districtRegionData>(*obj);
        }
    }

private:
    // inherited - County parsing was moved to parse so it only happens once during the data creation
};

#endif
