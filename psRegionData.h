#ifndef POLICEREGIONDATA_H
#define POLICEREGIONDATA_H

#include <vector>
#include <set>
#include <string>
#include <iomanip>
#include "regionData.h"

using namespace std;

class psRegionData : public regionData
{
public:
    static inline const set<string> includeColumns = set<string>{
        "name",
        "city",
        "state",
        "race",
        "signs_of_mental_illness",
        "armed",
        "body_camera"};

    psRegionData() : regionData()
    {
        this->propertyCounts["total_incidents"] = 0;
        this->propertyCounts["Ethnicities.White Alone"] = 0; // used
        this->propertyCounts["Ethnicities.White Alone, not Hispanic or Latino"] = 0;
        this->propertyCounts["Ethnicities.Black Alone"] = 0;
        this->propertyCounts["Ethnicities.Asian Alone"] = 0;
        this->propertyCounts["Ethnicities.American Indian and Alaska Native Alone"] = 0;
        this->propertyCounts["Ethnicities.Hispanic or Latino"] = 0;
        this->propertyCounts["Ethnicities.Two or More Races"] = 0;
        this->propertyCounts["Ethnicities.Native Hawaiian and Other Pacific Islander Alone"] = 0;
        this->propertyCounts["Ethnicities.Unspecified"] = 0;
        this->propertyCounts["signs_of_mental_illness"] = 0;
        this->propertyCounts["body_camera"] = 0;
        this->propertyCounts["armed"] = 0;
        this->propertyCounts["toy_weapon"] = 0;
    } // add

    void accept(class Visitor &v) override;

    void addName(string name)
    {
        names.push_back(name);
    }

    void aggregate(const psRegionData *newData);

    // complete these
    int getNumMentalI() const { return getPropertyCount("signs_of_mental_illness"); }; // total count “TRUE”
    int getUnArmedCount() const { return getPropertyCount("armed"); };                 // total count blank, ‘unarmed’
    int getArmedToy() const { return getPropertyCount("toy_weapon"); };                // total count ’toy weapon’
    int getNumBodyCam() const { return getPropertyCount("body_camera"); };             // total count “TRUE”
    int getNumberOfCases() const { return getPropertyCount("total_incidents"); };      // total number for a region

    // add any others needed

    friend std::ostream &operator<<(std::ostream &out, const psRegionData &PD);

protected:
    std::vector<string> names;
};

#endif