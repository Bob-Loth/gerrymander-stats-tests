#ifndef DEMOGREGIONDATA_H
#define DEMOGREGIONDATA_H

#include <vector>
#include <set>
#include <string>
#include <iomanip>
#include "regionData.h"
#include "parse.h"

using namespace std;

class demogRegionData : public regionData
{
public:
    static inline const set<string> includeColumns = set<string>{
        "County",
        "State",
        "Population.2020 Population",
        "Age.Percent Under 5 Years",
        "Age.Percent Under 18 Years",
        "Age.Percent 65 and Older",
        "Miscellaneous.Percent Female",
        "Ethnicities.White Alone",
        "Ethnicities.Black Alone",
        "Ethnicities.American Indian and Alaska Native Alone",
        "Ethnicities.Asian Alone",
        "Ethnicities.Native Hawaiian and Other Pacific Islander Alone",
        "Ethnicities.Two or More Races",
        "Ethnicities.Hispanic or Latino",
        "Ethnicities.White Alone, not Hispanic or Latino",
        "Miscellaneous.Veterans",
        "Miscellaneous.Foreign Born",
        "Housing.Households",
        "Housing.Housing Units",
        "Housing.Homeownership Rate",
        "Housing.Persons per Household",
        "Education.High School or Higher",
        "Education.Bachelor's Degree or Higher",
        "Income.Median Houseold Income",
    };

    demogRegionData() : regionData(), counties(1)
    {
        // init properties to 0;
    } // add

    demogRegionData(string inS) : regionData(inS), counties(1)
    {
        // init properties to 0;
    } // add

    demogRegionData(string inS, string inCounty) : regionData(inS, inCounty), counties(1)
    {
        // init properties to 0;
    } // add

    // add any others needed
    void aggregate(const demogRegionData* newData);

    void accept(class Visitor &v) override;

    friend std::ostream &operator<<(std::ostream &out, const demogRegionData &PD);

protected:
    int counties;
};

#endif