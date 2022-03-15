#ifndef VCOMBOKD_H
#define VCOMBOKD_H

#include "psData.h"
#include "psCombo.h"
#include "demogData.h"
#include "demogCombo.h"
#include "visitorCombine.h"
#include <memory>


class visitorCombineKeyDemog : public visitorCombine {
public:
    visitorCombineKeyDemog(string (*df)(unique_ptr<demogData>), std::map<string, string>& mapDemogToKey) {
      demogFunc = df;
      //probably want to assert the map is legit and store it
      assert(mapDemogToKey.size() > 0);
   }


  //just store based on function
	void visit(unique_ptr<demogData> obj) {
     //FILL IN
    }
    
    //store based on demog key
    void visit(unique_ptr<psData> obj) {
     //FILL IN
    }

    private:
      //the function pointers to the key functions
      string (*demogFunc)(unique_ptr<demogData>);

      //want a way to keep region to key for mapping between the two dataTypes

};

#endif
