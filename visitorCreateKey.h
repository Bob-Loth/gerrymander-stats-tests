#ifndef VCOMBOK_H
#define VCOMBOK_H

#include "psData.h"
#include "psCombo.h"
#include "demogData.h"
#include "demogCombo.h"
#include "visitorCombine.h"
#include <memory>


class visitorCreateKey : public Visitor {
public:
  visitorCreateKey(string (*df)(shared_ptr<demogData>), string (*psf)(shared_ptr<psData>)) {
      demogFunc = df;
      psFunc =psf; 
   }

	void visit(shared_ptr<demogData> obj) {
    //FILL IN
  }
    
  void visit(shared_ptr<psData> obj) {
    //FILL IN
  }

  //add getters as needed

    private:
      //the function pointers to the key functions
      string (*demogFunc)(shared_ptr<demogData>);
      string (*psFunc)(shared_ptr<psData>);

      //ADD data to map between location and key

};

#endif
