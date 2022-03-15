#ifndef VISITOR_H
#define VISITOR_H

#include "regionData.h"
#include "psRegionData.h"
#include "demogRegionData.h"
#include "districtRegionData.h"

/* a visitor can visit any of my specific data types */
class Visitor {
  public:
    virtual void visit(psRegionData* ps) = 0;
    virtual void visit(demogRegionData* d) = 0;
    virtual void visit(districtRegionData* d) = 0;
};
#endif

