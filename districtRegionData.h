#ifndef DISTRICTREGIONDATA_H
#define DISTRICTREGIONDATA_H

#include <vector>
#include <set>
#include <string>
#include <iomanip>
#include "regionData.h"
#include "parse.h"

using namespace std;

class districtRegionData : public regionData
{
public:
    static inline const set<string> includeColumns = set<string>{
        "2020HouseDemCount",
        "2020HouseRepCount"
    };

    districtRegionData() : regionData(), districts(1)
    {
        // init properties to 0;
    } // add

    districtRegionData(string inS) : regionData(inS), districts(1)
    {
        // init properties to 0;
    } // add

    districtRegionData(string inS, string inCounty) : regionData(inS, inCounty), districts(1)
    {
        // init properties to 0;
    } // add

    void setDistrictNum(std::string dnum) {
        districtNum = dnum;
    }

    std::string getDistrictNum() const {
        return districtNum;
    }

    // add any others needed
    void aggregate(const districtRegionData* newData);
    void accept(class Visitor &v) override;

    friend std::ostream &operator<<(std::ostream &out, const districtRegionData &PD);

    int districts;

protected:
    std::string districtNum;
};

#endif