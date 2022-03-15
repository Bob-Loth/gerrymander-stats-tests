#include "CountyMap.h"
#include <fstream>
#include "parse.h"

using namespace std;

std::string CountyMap::stripCounty(string inWord)
{
    string compareS = "County";
    /* some names include the word 'county' - strip */
    std::string::size_type i = inWord.find(compareS);
    if (i != std::string::npos)
    {
        inWord.erase(i - 1, compareS.length() + 1);
    }
    // apostrophe issue - strip will result in lack of rep - better solution?
    string symbol = "\'";
    i = inWord.find(symbol);
    if (i != std::string::npos)
    {
        inWord.erase(i - 1, symbol.length() + 1);
    }
    // strip Alaska designator
    string borough = " and Borough";
    i = inWord.find(borough);
    if (i != std::string::npos)
    {
        inWord.erase(i, borough.length() + 1);
    }
    string borough1 = "Borough";
    i = inWord.find(borough1);
    if (i != std::string::npos)
    {
        inWord.erase(i - 1, borough1.length() + 1);
    }
    string ca = "Census Area";
    i = inWord.find(ca);
    if (i != std::string::npos)
    {
        inWord.erase(i - 1, ca.length() + 1);
    }
    string p = "Parish";
    i = inWord.find(p);
    if (i != std::string::npos)
    {
        inWord.erase(i - 1, p.length() + 1);
    }
    return inWord;
}

//helper to create map from city to county
void CountyMap::read_csvCityCounty(std::string filename) {
     // Create an input filestream
     std::ifstream myFile(filename);

     // Make sure the file is open
     if(!myFile.is_open()) {
        throw std::runtime_error("Could not open file");
     }

     if(myFile.good()) {
        consumeColumnNames(myFile);

        // Helper vars
        std::string line;

        // Now read data, line by line and enter into the map
        while(std::getline(myFile, line)) {

          std::stringstream ss(line);

          std::string city = getField(ss);
          std::string junk1 = getField(ss);  //"city_ascii","city_alt"
          std::string junk0 = getField(ss);
          std::string state = getField(ss);
          std::string junk2 = getField(ss); //"state_name"
          std::string countyfips = getField(ss);
          std::string county = stripCounty(getField(ss));

          std::string countyKey = county+state;
          std::string cityKey = city+state;
          cityToCounty[cityKey] = countyID{county, std::stoi(countyfips)};
          countyToCounty[countyKey] = countyID{county, std::stoi(countyfips)};

          //cout << "line: " << line << endl;
          //cout << "pair (key, county): " << Key << ": " << county << " state " << state << " fip" << countyfips <<  endl;
        }

        // Close file
        myFile.close();
      }
}