/* ZJW helper code to get county ids - works in conjunction with uscities.csv file*/

#ifndef COUNTMAP_H
#define COUNTMAP_H

#include <map>
#include <iostream>

/* helper struct */
typedef struct countyID {
    std::string countyName;
    int countyfips;
} countyID;

class CountyMap {
  public:
	CountyMap(std::string filename) {
		noMatch = 0;
        read_csvCityCounty(filename);
	}

	void read_csvCityCounty(std::string fileIn);
	static std::string stripCounty(std::string inWord);

  public:
	std::map<std::string, countyID> cityToCounty;
	std::map<std::string, countyID> countyToCounty;
	int noMatch = 0;
};

#endif

