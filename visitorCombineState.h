#ifndef VCOMBOS_H
#define VCOMBOS_H


#include "visitorCombine.h"
#include <memory>

class visitorCombineState : public visitorCombine {
public:
    visitorCombineState() {}

	void visit(demogRegionData* obj) {
        //fill in
        if(allComboDemogData.find(obj->getState()) != allComboDemogData.end()) {
            allComboDemogData[obj->getState()]->aggregate(obj);
        } else {
            allComboDemogData[obj->getState()] = std::make_shared<demogRegionData>(*obj);
        }
    }
    
    //aggregate police shooting data by state
    void visit(psRegionData* obj) {
        if(allComboPoliceData.find(obj->getState()) != allComboPoliceData.end()) {
            allComboPoliceData[obj->getState()]->aggregate(obj);
        } else {
            allComboPoliceData[obj->getState()] = std::make_shared<psRegionData>(*obj);
        }
    }
    
    //aggregate district data by state
    void visit(districtRegionData* obj) {
        if(allComboDistrictData.find(obj->getState()) != allComboDistrictData.end()) {
            allComboDistrictData[obj->getState()]->aggregate(obj);
        } else {
            allComboDistrictData[obj->getState()] = std::make_shared<districtRegionData>(*obj);
        }
    }

    private:
       //inherited
};

#endif
