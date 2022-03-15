#include "psRegionData.h"
#include "visitor.h"

void psRegionData::accept(class Visitor &v)
{
    v.visit(this);
}

void psRegionData::aggregate(const psRegionData *newData)
{
    names.insert(std::end(names), std::begin(newData->names), std::end(newData->names));

    this->setRegion(regionData::combineRegionStrings(this->getRegion(), newData->getRegion()));
    this->setCounty(regionData::combineRegionStrings(this->getCounty(), newData->getCounty()));
    this->setState(regionData::combineRegionStrings(this->getState(), newData->getState()));

    if(this->countyFips != newData->countyFips) {
        this->countyFips = -1;
    }

    map<string, int>::const_iterator it;
    for (it = newData->propertyCounts.begin(); it != newData->propertyCounts.end(); it++)
    {
        if (propertyCounts.find(it->first) != propertyCounts.end())
        {
            propertyCounts[it->first] += it->second;
        }
        else
        {
            propertyCounts[it->first] = it->second;
        }
    }
}

string printProperty2(const psRegionData &DD, string title, string propName)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << "\n"
       << title;
    if (DD.getPropertyCount(propName) != 0)
    {
        ss << "percent: " << DD.getPropertyPercentage(propName, "total_incidents") << " ";
    }
    ss << "count: " << DD.getPropertyCount(propName);
    return ss.str();
}

/* print state data - as aggregate of all incidents in this state */
std::ostream &operator<<(std::ostream &out, const psRegionData &PD)
{
    out << "State Info: " << PD.stateName;
    out << "\nNumber of incidents: " << PD.getNumberOfCases();
    out << std::setprecision(2) << std::fixed;
    out << "\nIncidents involving unarmed or armed with toy weapons: " << (PD.getUnArmedCount() + PD.getArmedToy()) << ", " << ((PD.getUnArmedCount() + PD.getArmedToy()) / (float)PD.getNumberOfCases()) * 100 << "%";
    out << "\nIncidents involving mental illness: " << PD.getNumMentalI() << ", " << (PD.getNumMentalI() / (float)PD.getNumberOfCases()) * 100 << "%";
    out << "\nIncidents with body camera on: " << PD.getNumBodyCam() << ", " << (PD.getNumBodyCam() / (float)PD.getNumberOfCases()) * 100 << "%";
    out << "\nRacial demographics of state incidents: Racial Demographics Info:";
    out << printProperty2(PD, "% American Indian and Alaska Native ",
                          "Ethnicities.American Indian and Alaska Native Alone");
    out << printProperty2(PD, "% Asian American ",
                          "Ethnicities.Asian Alone");
    out << printProperty2(PD, "% Black/African American ",
                          "Ethnicities.Black Alone");
    out << printProperty2(PD, "% Hispanic or Latinx ",
                          "Ethnicities.Hispanic or Latino");
    out << printProperty2(PD, "% Native Hawaiian and Other Pacific Islander ",
                          "Ethnicities.Native Hawaiian and Other Pacific Islander Alone");
    out << printProperty2(PD, "% Two or More Races ",
                          "Ethnicities.Two or More Races");
    out << printProperty2(PD, "% White (inclusive) ",
                          "Ethnicities.White Alone");
    out << printProperty2(PD, "% White (nonHispanic) ",
                          "Ethnicities.White Alone");
    out << printProperty2(PD, "% Unspecified ",
                          "Ethnicities.Unspecified");
    out << "\ntotal Racial Demographic Count: " << PD.getNumberOfCases();
    return out;
}