#ifndef VCOMBO_H
#define VCOMBO_H

#include "psRegionData.h"
#include "demogRegionData.h"
#include "visitor.h"
#include <memory>
#include <map>

// The shared combo data
class visitorCombine : public Visitor
{
public:
  visitorCombine() {}

  // just a helper to print out various fields
  void printNCombo(int n)
  {
    int i = 0;
    cout << "All Combo data: " << endl;
    for (auto entry : allComboDemogData)
    {
      if (i < n)
      {
        cout << "name: " << entry.first << endl;
        // cout << "d: " << ((entry.second).get()->*f1)() << endl;
        cout << *(entry.second).get() << endl;
        if (allComboPoliceData.count(entry.first) > 0)
        {
          // cout << "ps: " << (allComboPoliceData[entry.first].get()->*f2)() << endl;
          cout << *(allComboPoliceData[entry.first].get()) << endl;
        }
      }
      i++;
    }
  }

  shared_ptr<demogRegionData> getRegionDataDemogData(string regionName) { return allComboDemogData[regionName]; }
  shared_ptr<psRegionData> getRegionDataPoliceData(string regionName) { return allComboPoliceData[regionName]; }
  shared_ptr<districtRegionData> getRegionDataDistrictData(string regionName) { return allComboDistrictData[regionName]; }

  std::map<string, shared_ptr<demogRegionData>> &getRegionDataDemog() { return allComboDemogData; }
  std::map<string, shared_ptr<psRegionData>> &getRegionDataPolice() { return allComboPoliceData; }
  std::map<string, shared_ptr<districtRegionData>> &getRegionDataDistrict() { return allComboDistrictData; }

protected:
  // combo maps
  std::map<string, shared_ptr<demogRegionData>> allComboDemogData;
  std::map<string, shared_ptr<psRegionData>> allComboPoliceData;
  std::map<string, shared_ptr<districtRegionData>> allComboDistrictData;
};

#endif
