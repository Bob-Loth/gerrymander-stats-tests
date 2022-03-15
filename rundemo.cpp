/*****************************************************************************
 * Example code modified from Mapnik rundemo.cpp (r727, license LGPL).
 * Renders the State of California using USGS state boundaries data.
 *****************************************************************************/

#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/save_map.hpp>
#include <mapnik/cairo_io.hpp>
#include <boost/optional/optional_io.hpp>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_image_util.hpp>
#endif

#include <iostream>

#include "regionData.h"
#include "parse.h"
#include "states.h"
#include "visitorAggregateLocations.h"
#include "visitorCombineCounty.h"
#include "visitorCombineState.h"
#include "visitorCollectStates.h"
#include "usRegionMap.h"
#include "gerryStats.h"

std::string getDistrictParseExpr(const regionData& r)
{
    const districtRegionData& dr = dynamic_cast<const districtRegionData&>(r);
    return "([STATE_ABBR]='" + r.getState() + "' and [CDFIPS]='" + dr.getDistrictNum() + "')";
}

std::string getStateParseExpr(const regionData& r)
{
    return "[STUSPS] = " + r.getState() + "";
}

std::string getRegionFips(const regionData& r)
{
    std::string expr = "([STATEFP] = \'";
    int stateCode = r.getCountyFips() / 1000;
    int countyCode = r.getCountyFips() % 1000;
    if (stateCode < 10)
    {
        expr += "0" + to_string(stateCode) + "\'";
    }
    else
    {
        expr += "" + to_string(stateCode) + "\'";
    }
    if (countyCode < 10)
        expr += " and [COUNTYFP] = \'00" + to_string(countyCode) + "\')";
    else if (countyCode < 100)
        expr += " and [COUNTYFP] = \'0" + to_string(countyCode) + "\')";
    else
        expr += " and [COUNTYFP] = \'" + to_string(countyCode) + "\')";
    return expr;
}

bool isValidPopCounty(const regionData& r)
{
    return (r.getCountyFips() != -1) && (r.getCounty() != "UNDEF") && (r.getCounty() != "MULTI") &&
           (r.hasProperty("Population.2020 Population"));
}

bool isValidIncidentCounty(const regionData& r)
{
    return (r.getCountyFips() != -1) && (r.getCounty() != "UNDEF") && (r.getCounty() != "MULTI") &&
           (r.hasProperty("total_incidents"));
}

bool isValidDistrict(const regionData& r)
{
    return (r.hasProperty("2020HouseDemCount") && r.hasProperty("2020HouseDemCount") &&
            (r.getPropertyCount("2020HouseDemCount") != 0 || r.getPropertyCount("2020HouseRepCount") != 0));
}

int main(int argc, char** argv)
{
    using namespace mapnik;

    std::vector<color> rwbColorMap = {color(0, 0, 255),
                                      color(60, 60, 255),
                                      color(120, 120, 255),
                                      color(180, 180, 255),
                                      color(240, 240, 255),
                                      color(255, 240, 240),
                                      color(255, 180, 180),
                                      color(255, 120, 120),
                                      color(255, 60, 60),
                                      color(255, 0, 0)};
    std::vector<color> colorMapFill = {color(91, 80, 235),
                                       color(95, 245, 155),
                                       color(128, 235, 96),
                                       color(235, 235, 75),
                                       color(245, 213, 91),
                                       color(223, 170, 94),
                                       color(245, 134, 91),
                                       color(235, 91, 101)};
    std::vector<color> colorMapOutline = {color(0, 0, 0), color(235, 91, 101)};

    // read in a csv file and create a vector of objects representing each counties data
    std::vector<unique_ptr<regionData>> pileOfData =
      read_csv("demo/gerrymander-stats-tests/county_demographics.csv", DEMOG);

    std::vector<unique_ptr<regionData>> thePoliceData =
      read_csv("demo/gerrymander-stats-tests/fatal-police-shootings-data-Q.csv", POLICE);

    std::vector<unique_ptr<regionData>> theDistrictData =
      read_csv("demo/gerrymander-stats-tests/districtCountInfo.csv", DISTRICT);

    pileOfData.insert(std::end(pileOfData),
                      std::make_move_iterator(thePoliceData.begin()),
                      std::make_move_iterator(thePoliceData.end()));

    pileOfData.insert(std::end(pileOfData),
                      std::make_move_iterator(theDistrictData.begin()),
                      std::make_move_iterator(theDistrictData.end()));

    visitorCombineCounty combineCounties;
    visitorCombineState combineStates;
    visitorCollectStates stateCollections;
    for (const auto& obj : pileOfData)
    {
        obj->accept(stateCollections);
        obj->accept(combineCounties);
        obj->accept(combineStates);
    }

    std::map<string, shared_ptr<psRegionData>> psCounties = combineCounties.getRegionDataPolice();
    std::map<string, shared_ptr<demogRegionData>> demogCounties = combineCounties.getRegionDataDemog();
    std::map<string, shared_ptr<demogRegionData>> demogStates = combineStates.getRegionDataDemog();
    std::map<string, std::vector<districtRegionData*>> districtCollections = stateCollections.getDistrictValues();

    // Vote shares
    visitorAggregateLocations aggVoteShares(isValidDistrict, getDistrictParseExpr, [](const regionData& r) {
        return r.getPropertyCount("2020HouseRepCount") /
               (float)(r.getPropertyCount("2020HouseDemCount") + r.getPropertyCount("2020HouseRepCount"));
    });
    visitorAggregateLocations aggEfficiencyGap(
      [](const regionData& r) { return true; },
      getStateParseExpr,
      [&](const regionData& r) {
          vector<int> demCounts;
          vector<int> repCounts;
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(demCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseDemCount"); });
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(repCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseRepCount"); });
          return getEfficiencyGap(demCounts, repCounts);
      });

    visitorAggregateLocations aggPartisanBias(
      [](const regionData& r) { return true; },
      getStateParseExpr,
      [&](const regionData& r) {
          vector<int> demCounts;
          vector<int> repCounts;
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(demCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseDemCount"); });
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(repCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseRepCount"); });
          if(demCounts.size() == 0) {
              return 0.0;
          }
          return getPartisanBias(demCounts, repCounts);
      });
    visitorAggregateLocations aggMeanMedianScores(
      [](const regionData& r) { return true; },
      getStateParseExpr,
      [&](const regionData& r) {
          vector<int> demCounts;
          vector<int> repCounts;
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(demCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseDemCount"); });
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(repCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseRepCount"); });
          if(demCounts.size() == 0) {
              return 0.0;
          }
          vector<double> demShares;
          for (int i = 0; i < demCounts.size(); i++)
          {
              if(demCounts[i] != 0 || repCounts[i] != 0)
              {
                demShares.push_back(demCounts[i] / (double)(repCounts[i] + demCounts[i]));
              }
          }
          return getMeanMedianScores(demShares);
      });
    visitorAggregateLocations aggDeclinationAngle(
      [](const regionData& r) { return true; },
      getStateParseExpr,
      [&](const regionData& r) {
          vector<int> demCounts;
          vector<int> repCounts;
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(demCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseDemCount"); });
          transform(districtCollections[r.getState()].begin(),
                    districtCollections[r.getState()].end(),
                    std::back_inserter(repCounts),
                    [](districtRegionData* drd) { return drd->getPropertyCount("2020HouseRepCount"); });
          if(demCounts.size() == 0) {
              return 0.0;
          }
          vector<double> demShares;
          for (int i = 0; i < demCounts.size(); i++)
          {
              if(demCounts[i] != 0 || repCounts[i] != 0)
              {
                demShares.push_back(demCounts[i] / (double)(repCounts[i] + demCounts[i]));
              }
          }
          cout << r.getState()  << ": " << demCounts.size() << " - " << repCounts.size() << endl;
          return getDeclinationAngle(demShares);
      });

    for (const auto& obj : pileOfData)
    {
        obj->accept(aggVoteShares);
    }
    for (const auto& obj : demogStates)
    {
        obj.second->accept(aggEfficiencyGap);
        obj.second->accept(aggPartisanBias);
        obj.second->accept(aggMeanMedianScores);
        obj.second->accept(aggDeclinationAngle);
    }

    usRegionMap eg("EfficiencyGap");
    usRegionMap pb("PartisanBias");
    usRegionMap mms("MeanMedianScores");
    usRegionMap da("DeclinationAngle");

    // District Coloring --------------------------------------------------
    eg.addParsedMapLayer(
      "DistrictColoring",
      aggVoteShares,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::polygon_symbolizer poly_sym;
          put(poly_sym, mapnik::keys::fill, c);
          return (symbolizer)poly_sym;
      },
      "demo/data/cd117");
    pb.addParsedMapLayer(
      "DistrictColoring",
      aggVoteShares,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::polygon_symbolizer poly_sym;
          put(poly_sym, mapnik::keys::fill, c);
          return (symbolizer)poly_sym;
      },
      "demo/data/cd117");
    mms.addParsedMapLayer(
      "DistrictColoring",
      aggVoteShares,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::polygon_symbolizer poly_sym;
          put(poly_sym, mapnik::keys::fill, c);
          return (symbolizer)poly_sym;
      },
      "demo/data/cd117");
    da.addParsedMapLayer(
      "DistrictColoring",
      aggVoteShares,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::polygon_symbolizer poly_sym;
          put(poly_sym, mapnik::keys::fill, c);
          return (symbolizer)poly_sym;
      },
      "demo/data/cd117");

    // District lines --------------------------------------------------
    mapnik::feature_type_style districtlines_style1;
    {
        rule r;
        {
            {
                mapnik::polygon_symbolizer poly_sym;
            }
            mapnik::line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(0, 0, 0));
            put(line_sym, keys::stroke_width, 1.0f);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        districtlines_style1.add_rule(std::move(r));
    }
    eg.addSimpleMapLayer("DistrictLines", "demo/data/cd117", std::move(districtlines_style1));

    mapnik::feature_type_style districtlines_style2;
    {
        rule r;
        {
            {
                mapnik::polygon_symbolizer poly_sym;
            }
            mapnik::line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(0, 0, 0));
            put(line_sym, keys::stroke_width, 1.0f);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        districtlines_style2.add_rule(std::move(r));
    }
    pb.addSimpleMapLayer("DistrictLines", "demo/data/cd117", std::move(districtlines_style2));

    mapnik::feature_type_style districtlines_style3;
    {
        rule r;
        {
            {
                mapnik::polygon_symbolizer poly_sym;
            }
            mapnik::line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(0, 0, 0));
            put(line_sym, keys::stroke_width, 1.0f);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        districtlines_style3.add_rule(std::move(r));
    }
    mms.addSimpleMapLayer("DistrictLines", "demo/data/cd117", std::move(districtlines_style3));
    mapnik::feature_type_style districtlines_style4;
    {
        rule r;
        {
            {
                mapnik::polygon_symbolizer poly_sym;
            }
            mapnik::line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(0, 0, 0));
            put(line_sym, keys::stroke_width, 1.0f);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        districtlines_style4.add_rule(std::move(r));
    }
    da.addSimpleMapLayer("DistrictLines", "demo/data/cd117", std::move(districtlines_style4));

    // State Line Coloring -------------------------------------------------
    cout << "eg\n";
    eg.addParsedMapLayer(
      "StateBorderColoring",
      aggEfficiencyGap,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::line_symbolizer line_sym;
          put(line_sym, keys::stroke, c);
          put(line_sym, keys::stroke_width, 6.0f);
          put(line_sym, keys::stroke_linecap, ROUND_CAP);
          put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
          return (symbolizer)line_sym;
      },
      "demo/data/cb_2018_us_state_20m");
    cout << "pb\n";
    pb.addParsedMapLayer(
      "StateBorderColoring",
      aggPartisanBias,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::line_symbolizer line_sym;
          put(line_sym, keys::stroke, c);
          put(line_sym, keys::stroke_width, 6.0f);
          put(line_sym, keys::stroke_linecap, ROUND_CAP);
          put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
          return (symbolizer)line_sym;
      },
      "demo/data/cb_2018_us_state_20m");
    cout << "mms\n";
    mms.addParsedMapLayer(
      "StateBorderColoring",
      aggMeanMedianScores,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::line_symbolizer line_sym;
          put(line_sym, keys::stroke, c);
          put(line_sym, keys::stroke_width, 6.0f);
          put(line_sym, keys::stroke_linecap, ROUND_CAP);
          put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
          return (symbolizer)line_sym;
      },
      "demo/data/cb_2018_us_state_20m");
    cout << "da\n";
    da.addParsedMapLayer(
      "StateBorderColoring",
      aggDeclinationAngle,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::line_symbolizer line_sym;
          put(line_sym, keys::stroke, c);
          put(line_sym, keys::stroke_width, 6.0f);
          put(line_sym, keys::stroke_linecap, ROUND_CAP);
          put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
          return (symbolizer)line_sym;
      },
      "demo/data/cb_2018_us_state_20m");

    eg.saveToJPG();
    pb.saveToJPG();
    mms.saveToJPG();
    da.saveToJPG();
}
