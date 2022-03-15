/* helper routines to read out csv data */
#include "parse.h"

/* helper to strip out quotes from a string */
string stripQuotes(std::string temp)
{
    temp.erase(remove(temp.begin(), temp.end(), '\"'), temp.end());
    return temp;
}

/* helper: get field from string stream */
/* assume field has quotes for CORGIS */
string getField(std::stringstream& ss)
{
    string data, junk;
    // ignore the first quotes
    std::getline(ss, junk, '\"');
    // read the data (not to comma as some data includes comma (Hospital names))
    std::getline(ss, data, '\"');
    // read to comma final comma (to consume and prep for next)
    std::getline(ss, junk, ',');
    // data includes final quote (see note line 18)
    return stripQuotes(data);
}

/* helper: read out column names for CSV file */
unique_ptr<vector<Column>> consumeColumnNames(std::ifstream& myFile)
{
    std::string line;
    std::string colname, junk;

    // Extract the first line in the file
    std::getline(myFile, line);

    // Create a stringstream from line
    std::stringstream ss(line);

    auto columns = make_unique<vector<Column>>();

    // Read the column names (for debugging)
    // Extract each column name for debugging
    // have to use quotes to seperate column names because commas appear in column names:
    //    "Ethnicities.White Alone, Ethnicities.White Alone"
    int c = 0;
    while (std::getline(ss, junk, '\"'))
    {
        std::getline(ss, colname, '\"');
        std::getline(ss, junk, ',');
        // cout << "cname: " << colname << endl;
        columns->push_back(Column(colname, c));
        c++;
    }
    return columns;
}

/* Read one line from a CSV file for county demographic data specifically
 **only uses cols pointer for the duration of this function**
 */
unique_ptr<regionData>
  readCSVLineDistrict(std::string theLine, vector<Column>* cols, const set<string>& keep_cols, CountyMap& cm)
{
    std::stringstream ss(theLine);

    auto colIttr = cols->begin();
    string dname = getField(ss);
    colIttr++;
    string dId = getField(ss);
    colIttr++;
    string state = dId.substr(0,2);
    string district = dId.substr(3,5);

    auto demData = make_unique<districtRegionData>(state);
    demData->setDistrictNum(district);

    while (colIttr != cols->end())
    {
        string field = getField(ss);
        // strip quotes
        string cName = colIttr->colName;
        if (keep_cols.find(cName) != keep_cols.end())
        {
            demData->addProperty(cName, stoi(field));
        }
        colIttr++;
    }

    return demData;
}

/* Read one line from a CSV file for county demographic data specifically
 **only uses cols pointer for the duration of this function**
 */
unique_ptr<regionData>
  readCSVLineDemog(std::string theLine, vector<Column>* cols, const set<string>& keep_cols, CountyMap& cm)
{
    std::stringstream ss(theLine);

    auto colIttr = cols->begin();
    string name = getField(ss);
    colIttr++;
    string state = getField(ss);
    colIttr++;
    int pop = 1;
    int housingPop = 1;
    int housingUnits = 1;
    float pph = 0;
    float hr = 0;

    std::map<std::string, float> popscaling;

    auto demData = make_unique<demogRegionData>(state, name);
    demData->setRegion(name);
    std::string county = CountyMap::stripCounty(name);

    demData->setCounty(county);
    if (cm.countyToCounty.find(county + state) != cm.countyToCounty.end())
    {
        demData->setCounty(county);
        demData->setCountyFips(cm.countyToCounty[county + state].countyfips);
    }
    else
    {
        string mun = "Municipality";
        int i = county.find(mun);
        if (i != std::string::npos)
        {
            county.erase(i - 1, county.length() + 1);
        }
        string city = "city";
        i = county.find(city);
        if (i != std::string::npos)
        {
            county.erase(i - 1, city.length() + 1);
        }
        city = "City";
        i = county.find(city);
        if (i != std::string::npos)
        {
            county.erase(i - 1, city.length() + 1);
        }
        if (cm.countyToCounty.find(county + state) != cm.countyToCounty.end())
        {
            demData->setCounty(county);
            demData->setCountyFips(cm.countyToCounty[county + state].countyfips);
        }
        else
        {
            // cerr << "demog county unmatched: " << county << state << endl;
        }
    }

    while (colIttr != cols->end())
    {
        string field = getField(ss);
        // strip quotes
        string cName = colIttr->colName;
        if (keep_cols.find(cName) != keep_cols.end())
        {
            // cout << cName << endl;
            if (cName == "Population.2020 Population")
            {
                pop = stoi(field);
                demData->addProperty(cName, pop);
            }
            else if (cName == "Housing.Households")
            {
                housingPop = stoi(field);
                demData->addProperty(cName, housingPop);
            }
            else if (cName == "Housing.Housing Units")
            {
                housingUnits = stoi(field);
                demData->addProperty(cName, housingUnits);
            }
            else if (cName == "Housing.Homeownership Rate")
            {
                // special case as this is households not population
                hr = stod(field);
            }
            else if (cName == "Housing.Persons per Household")
            {
                // special case as this is households not population
                pph = stod(field);
            }
            else if (field.find('.') != std::string::npos)
            {
                // this is a double property, these are percentages so we have to divide by 100
                popscaling[cName] = stod(field);
            }
            else
            {
                // this is an int property
                demData->addProperty(cName, stoi(field));
            }
        }
        colIttr++;
    }
    demData->addProperty("Housing.Persons per Household", (int)round(pph * housingPop));
    demData->addProperty("Housing.Homeownership Rate", (int)round((hr / 100) * housingPop));

    std::map<std::string, float>::iterator it;
    for (it = popscaling.begin(); it != popscaling.end(); it++)
    {
        demData->addProperty(it->first, (int)round((it->second / 100) * pop));
    }

    return demData;
}

// read one line of police data
unique_ptr<regionData>
  readCSVLinePolice(std::string theLine, vector<Column>* cols, const set<string>& keep_cols, CountyMap& cm)
{
    std::stringstream ss(theLine);
    auto colIttr = cols->begin();

    auto polData = make_unique<psRegionData>();

    polData->addProperty("total_incidents", 1);

    while (colIttr != cols->end())
    {
        string field = getField(ss);
        // strip quotes
        string cName = colIttr->colName;
        if (keep_cols.find(cName) != keep_cols.end())
        {
            if (cName == "city")
            {
                polData->setRegion(field);
            }
            else if (cName == "state")
            {
                polData->setState(field);
            }
            else if (cName == "name")
            {
                polData->addName(field);
            }
            else if (cName == "race")
            {
                if (field == "")
                {
                    polData->addProperty("Ethnicities.Unspecified", 1);
                }
                else if (field == "W")
                {
                    polData->addProperty("Ethnicities.White Alone", 1);
                }
                else if (field == "B")
                {
                    polData->addProperty("Ethnicities.Black Alone", 1);
                }
                else if (field == "A")
                {
                    polData->addProperty("Ethnicities.Asian Alone", 1);
                }
                else if (field == "N")
                {
                    polData->addProperty("Ethnicities.American Indian and Alaska Native Alone", 1);
                }
                else if (field == "H")
                {
                    polData->addProperty("Ethnicities.Hispanic or Latino", 1);
                }
                else if (field == "O")
                {
                    polData->addProperty("Ethnicities.Unspecified", 1);
                }
            }
            else if (cName == "signs_of_mental_illness")
            {
                polData->addProperty("signs_of_mental_illness", field == "TRUE" ? 1 : 0);
            }
            else if (cName == "body_camera")
            {
                polData->addProperty("body_camera", field == "TRUE" ? 1 : 0);
            }
            else if (cName == "armed")
            {
                polData->addProperty("armed", (field == "unarmed" || field == "") ? 1 : 0);
                polData->addProperty("toy_weapon", (field.find("toy") != std::string::npos) ? 1 : 0);
            }
        }
        colIttr++;
    }

    // Strip township
    string townsh = "Township";
    string region = polData->getRegion();
    int i = region.find(townsh);
    if (i != std::string::npos)
    {
        region.erase(i - 1, townsh.length() + 1);
    }
    if (cm.cityToCounty.find(region + polData->getState()) != cm.cityToCounty.end())
    {
        countyID county = cm.cityToCounty[region + polData->getState()];
        polData->setCounty(CountyMap::stripCounty(county.countyName));
        polData->setCountyFips(county.countyfips);
    }
    else
    {
        std::string county = CountyMap::stripCounty(region);
        polData->setCounty(county);
        // If we can strip it it was a county, parish, or borough
        if (county.length() != region.length() &&
            cm.countyToCounty.find(county + polData->getState()) != cm.countyToCounty.end())
        {
            polData->setCounty(county);
            polData->setCountyFips(cm.countyToCounty[county + polData->getState()].countyfips);
        }
        else
        {
            // cerr << "ps county unmatched: " << county << polData->getState() << endl;
        }
    }

    return polData;
}

std::vector<unique_ptr<regionData>> read_csv(std::string filename, typeFlag fileType)
{
    std::vector<unique_ptr<regionData>> theData;

    // Create an input filestream
    std::ifstream myFile(filename);

    // Make sure the file is open
    if (!myFile.is_open())
    {
        throw std::runtime_error("Could not open file");
    }

    if (myFile.good())
    {
        auto columns = consumeColumnNames(myFile);
        // Helper vars
        std::string line;
        CountyMap cm = CountyMap(
          "demo/gerrymander-stats-tests/uscities.csv");

        // Now read data, line by line and create demographic dataobject
        while (std::getline(myFile, line))
        {
            if (fileType == POLICE)
            {
                theData.push_back(readCSVLinePolice(line, columns.get(), psRegionData::includeColumns, cm));
            }
            else if (fileType == DEMOG)
            {
                theData.push_back(readCSVLineDemog(line, columns.get(), demogRegionData::includeColumns, cm));
            }
            else if (fileType == DISTRICT)
            {
                theData.push_back(readCSVLineDistrict(line, columns.get(), districtRegionData::includeColumns, cm));
            }
            else
            {
                cout << "ERROR - unknown file type" << endl;
                exit(0);
            }
        }

        // Close file
        myFile.close();
    }

    return theData;
}