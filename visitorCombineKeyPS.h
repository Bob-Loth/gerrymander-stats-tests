#ifndef VCOMBOKP_H
#define VCOMBOKP_H

#include "psData.h"
#include "psCombo.h"
#include "demogData.h"
#include "demogCombo.h"
#include "visitorCombine.h"
#include <memory>


class visitorCombineKeyPS : public visitorCombine {
public:
    visitorCombineKeyPS(string (*psf)(shared_ptr<psData>), std::map<string, string>& mapPSToKey) {
      psFunc = psf;
      //probably want to assert the map is legit and store it
      assert(mapPSToKey.size() > 0);
   }

  //based on stored key
	void visit(shared_ptr<demogData> obj) {
    //FILL IN
      
  }
    
  //store based on function
  void visit(shared_ptr<psData> obj) {
    //FILL IN
  }

  private:
    //the function pointers to the key functions
    string (*psFunc)(shared_ptr<psData>);

    //data to help region to key for mapping between the two dataTypes

};

#endif
