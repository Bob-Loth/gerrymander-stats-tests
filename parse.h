
#ifndef PARSE_H
#define PARSE_H

#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <sstream> // std::stringstream
#include <memory>
#include <math.h>
#include <algorithm>
#include <memory>
#include "regionData.h"
#include "demogRegionData.h"
#include "districtRegionData.h"
#include "psRegionData.h"
#include "CountyMap.h"


/*static functions to help parse CSV data */

/* LAB01 starter - replace with your revised version! */

/* For future assignments when we read different types of data */
enum typeFlag {
	DEMOG = 0,
	HOSPITAL = 1,
	POLICE = 2,
	DISTRICT = 3
};

// represent a column of data
struct Column {
	Column(string colN, int c) : colName(colN), col(c) {}
	string colName;
	int col;
};

/* helper to strip out quotes from a string */
string stripQuotes(std::string temp) ;

/* helper: get field from string stream */
/* assume field has quotes for CORGIS */
string getField(std::stringstream &ss);

//WP data no quotes
string getFieldNQ(std::stringstream &ss);

/* helper: read out column names for CSV file */
unique_ptr<vector<Column>> consumeColumnNames(std::ifstream &myFile);

// Read one line from a CSV file for county demographic data specifically
unique_ptr<regionData> readCSVLineDemog(std::string theLine, vector<Column>* cols, const set<string>& keep_cols, CountyMap& cm);

//read from a CSV file (for a given data type) return a vector of the data
std::vector<unique_ptr<regionData>> read_csv(std::string filename, typeFlag fileType);

// Read one line from a CSV file for police shooting data specifically
unique_ptr<regionData> readCSVLinePolice(std::string theLine, vector<Column>* cols, const set<string>& keep_cols, CountyMap& cm);

// Read one line from a CSV file for district data specifically
unique_ptr<regionData> readCSVLineDistrict(std::string theLine, vector<Column>* cols, const set<string>& keep_cols, CountyMap& cm);

#endif