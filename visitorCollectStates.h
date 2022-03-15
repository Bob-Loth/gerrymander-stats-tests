#ifndef VCS_H
#define VCS_H

#include "psRegionData.h"
#include "demogRegionData.h"
#include "districtRegionData.h"
#include "visitor.h"
#include "parse.h"
#include <memory>
#include <fstream>
#include <cassert>

class visitorCollectStates : public Visitor
{
  public:
    // store demographic data by county name
    void visit(demogRegionData* obj)
    {
        demogValues[obj->getState()].push_back(obj);
    }

    // aggregate police shooting data by county
    void visit(psRegionData* obj)
    {
        psValues[obj->getState()].push_back(obj);
    }
    
    // aggregate police shooting data by county
    void visit(districtRegionData* obj)
    {
        districtValues[obj->getState()].push_back(obj);
    }

    const std::map<std::string, std::vector<psRegionData*>>& getPsValues() const { return psValues; }
    const std::map<std::string, std::vector<demogRegionData*>>& getDemogValues() const { return demogValues; }
    const std::map<std::string, std::vector<districtRegionData*>>& getDistrictValues() const { return districtValues; }

  private:
    std::map<std::string, std::vector<psRegionData*>> psValues;
    std::map<std::string, std::vector<demogRegionData*>> demogValues;
    std::map<std::string, std::vector<districtRegionData*>> districtValues;
};

#endif
