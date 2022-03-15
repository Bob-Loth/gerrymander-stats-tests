#include "districtRegionData.h"
#include "visitor.h"

void districtRegionData::accept(class Visitor &v)
{
    v.visit(this);
}

void districtRegionData::aggregate(const districtRegionData *newData)
{
    this->districts += newData->districts;
    this->setRegion(regionData::combineRegionStrings(this->getRegion(), newData->getRegion()));
    this->setCounty(regionData::combineRegionStrings(this->getCounty(), newData->getCounty()));
    this->setState(regionData::combineRegionStrings(this->getState(), newData->getState()));
    this->districtNum = "UNDEF";
    this->estDemVoteShare = 0;
    this->mvap = 0;
}

/* print state demographic data */
std::ostream &operator<<(std::ostream &out, const districtRegionData &DD)
{
    out << "District Info: " << DD.getState() << endl;

    return out;
}