#ifndef REGION_H
#define REGION_H

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <map>

using namespace std;

/* very general data type for any kind of regional data*/
class regionData
{
public:

    regionData() : stateName("UNDEF"), countyName("UNDEF"), regionName("UNDEF"), countyFips(-1) {}
    // fill in constructors
    regionData(string state) : stateName(state), countyName("UNDEF"), regionName("UNDEF")
    {
    }

    regionData(string state, string county) : stateName(state), countyName(county), regionName("UNDEF")
    {
    }

    virtual void accept(class Visitor &v) = 0;

    string getRegion() const { return regionName; }
    string getState() const { return stateName; }
    string getCounty() const { return countyName; }
    int getCountyFips() const { return countyFips; }
    void setRegion(string r) { regionName = r; }
    void setState(string s) { stateName = s; }
    void setCounty(string c) { countyName = c; }
    void setCountyFips(int f) { countyFips = f; }

    void addProperty(string propName, int propCount)
    {
        this->propertyCounts[propName] = propCount;
    }

    bool hasProperty(std::string prop) const
    {
        return this->propertyCounts.count(prop);
    }

    int getPropertyCount(std::string prop) const
    {
        try
        {
            return this->propertyCounts.at(prop);
        }
        catch (std::out_of_range &e)
        {
            cout << "Parameter not found: " << prop << "\n";
            return -1;
        }
    }

    float getPropertyPercentage(std::string prop, std::string populationProp) const
    {
        int propPopulation = getPropertyCount(populationProp);
        int propValue = getPropertyCount(prop);
        return propPopulation == 0 ? 0 : (((float)propValue) / (float)(propPopulation)) * 100;
    }

protected:

    static string combineRegionStrings(string s1, string s2) {
        return s1 == s2 ? s1 : "MULTI";
    }

    std::string regionName;
    std::string stateName;
    std::string countyName;
    int countyFips;
    std::map<string, int> propertyCounts;
};
#endif
