#ifndef VAGGL_H
#define VAGGL_H

#include "psRegionData.h"
#include "demogRegionData.h"
#include "visitor.h"
#include "parse.h"
#include <memory>
#include <fstream>
#include <cassert>

class visitorAggregateLocations : public Visitor
{
  public:

    visitorAggregateLocations(function<bool (const regionData&)> sV,
                              function<std::string (const regionData&)> gR,
                              function<float (const regionData&)> gV)
        : shouldVisit(sV)
        , getRegion(gR)
        , getValue(gV)
    {}

    // store demographic data by county name
    void visit(demogRegionData* obj)
    {
        if (shouldVisit(*obj))
        {
            std::string region = getRegion(*obj);
            if (locationValues.find(region) != locationValues.end())
            {
                locationValues[region] += getValue(*obj);
            }
            else
            {
                locationValues[region] = getValue(*obj);
            }
        }
    }

    // aggregate police shooting data by county
    void visit(psRegionData* obj)
    {
        if (shouldVisit(*obj))
        {
            std::string region = getRegion(*obj);
            if (locationValues.find(region) != locationValues.end())
            {
                locationValues[region] += getValue(*obj);
            }
            else
            {
                locationValues[region] = getValue(*obj);
            }
        }
    }
    
    // aggregate police shooting data by county
    void visit(districtRegionData* obj)
    {
        if (shouldVisit(*obj))
        {
            std::string region = getRegion(*obj);
            if (locationValues.find(region) != locationValues.end())
            {
                locationValues[region] += getValue(*obj);
            }
            else
            {
                locationValues[region] = getValue(*obj);
            }
        }
    }

    const std::map<std::string, float>& getLocationValues() const { return locationValues; }

  private:
    std::map<std::string, float> locationValues;
    function<bool (const regionData&)> shouldVisit;
    function<float (const regionData&)> getValue;
    function<std::string (const regionData&)> getRegion;
};

#endif
