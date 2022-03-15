#include "demogRegionData.h"
#include "visitor.h"

void demogRegionData::accept(class Visitor &v)
{
    v.visit(this);
}

void demogRegionData::aggregate(const demogRegionData *newData)
{
    this->counties += newData->counties;
    this->setRegion(regionData::combineRegionStrings(this->getRegion(), newData->getRegion()));
    this->setCounty(regionData::combineRegionStrings(this->getCounty(), newData->getCounty()));
    this->setState(regionData::combineRegionStrings(this->getState(), newData->getState()));

    map<string, int>::const_iterator it;
    for (it = newData->propertyCounts.begin(); it != newData->propertyCounts.end(); it++)
    {
        if(propertyCounts.find(it->first) != propertyCounts.end()) {
            propertyCounts[it->first] += it->second;
        } else {
            propertyCounts[it->first] = it->second;
        }
    }
}

// Helper function to print a property of state demographic data
string printProperty(const demogRegionData &DD, string percentPre,
                     string countPre, string propName)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << "\n"
       << percentPre << DD.getPropertyPercentage(propName, "Population.2020 Population") << countPre << DD.getPropertyCount(propName);
    return ss.str();
}

// Helper function to print a property of state demographic data
// required because of how inconsistent the output is
string printProperty2(const demogRegionData &DD, string title, string propName)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << "\n"
       << title;
    if (DD.getPropertyCount(propName) != 0)
    {
        ss << "percent: " << DD.getPropertyPercentage(propName, "Population.2020 Population") << " ";
    }
    ss << "count: " << DD.getPropertyCount(propName);
    return ss.str();
}

/* print state demographic data */
std::ostream &operator<<(std::ostream &out, const demogRegionData &DD)
{
    out << "State Info: " << DD.getState() << endl;
    out << "Number of Counties: " << DD.counties;
    out << "\nTotal population: " << DD.getPropertyCount("Population.2020 Population");
    // consider re-writing with getters....
    out << "\nRacial Demographics Info: ";
    out << printProperty2(DD, "% American Indian and Alaska Native ",
                          "Ethnicities.American Indian and Alaska Native Alone");
    out << printProperty2(DD, "% Asian American ",
                          "Ethnicities.Asian Alone");
    out << printProperty2(DD, "% Black/African American ",
                          "Ethnicities.Black Alone");
    out << printProperty2(DD, "% Hispanic or Latinx ",
                          "Ethnicities.Hispanic or Latino");
    out << printProperty2(DD, "% Native Hawaiian and Other Pacific Islander ",
                          "Ethnicities.Native Hawaiian and Other Pacific Islander Alone");
    out << printProperty2(DD, "% Two or More Races ",
                          "Ethnicities.Two or More Races");
    out << printProperty2(DD, "% White (inclusive) ",
                          "Ethnicities.White Alone");
    out << printProperty2(DD, "% White (nonHispanic) ",
                          "Ethnicities.White Alone, not Hispanic or Latino");
    out << "\ntotal Racial Demographic Count: " << DD.getPropertyCount("Population.2020 Population");

    out << "\n--Population Age info:";
    out << printProperty(DD, "(under 5): ",
                         "% and total: ", "Age.Percent Under 5 Years");
    out << printProperty(DD, "(under 18): ",
                         "% and total: ", "Age.Percent Under 18 Years");
    out << printProperty(DD, "(over 65): ",
                         "% and total: ", "Age.Percent 65 and Older");

    out << "\n--Population other demographics";
    out << printProperty(DD, "(female): ",
                         "% and total: ", "Miscellaneous.Percent Female");
    out << printProperty(DD, "(foreign born): ",
                         "% and total: ", "Miscellaneous.Foreign Born");
    out << printProperty(DD, "(veterans): ",
                         "% and total: ", "Miscellaneous.Veterans");

    out << "\n--County housing/economy info:";
    out << "(avg. persons per house): " << DD.getPropertyCount("Housing.Persons per Household") / (float)DD.getPropertyCount("Housing.Households")
        << " and total: " << DD.getPropertyCount("Housing.Persons per Household");
    out << printProperty(DD, "(home ownership): ",
                         "% and total: ", "Housing.Homeownership Rate");

    out << "\n(number households): " << DD.getPropertyCount("Housing.Households");
    out << "\n(medium income): $" << DD.getPropertyCount("Income.Median Houseold Income");

    out << "\n--County education:";
    out << printProperty(DD, "(Bachelor or more): ",
                         "% and total: ", "Education.Bachelor's Degree or Higher");
    out << printProperty(DD, "(high school or more): ",
                         "% and total: ", "Education.High School or Higher");

    return out;
}