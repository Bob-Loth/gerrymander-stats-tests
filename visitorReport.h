#ifndef VISITREP_H
#define VISITREP_H

#include "psRegionData.h"
#include "demogRegionData.h"
#include "visitor.h"

class visitorReport : public Visitor {
public:
    visitorReport() : numVisited(0) {}

    void visit(demogRegionData* e) override {
        cout << *e << endl;
        numVisited++;
    }
    
    void visit(psRegionData* e) override {
        cout << *e << endl;
        numVisited++;
    }

    private:
    	int numVisited;
};

#endif