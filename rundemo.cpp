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
#include "usRegionMap.h"

std::string getDistrictParseExpr(const regionData& r)
{
    const districtRegionData& dr = dynamic_cast<const districtRegionData&>(r);
    return "([STATE_ABBR]='" + r.getState() + "' and [CDFIPS]='" + dr.getDistrictNum() + "')";
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

    visitorCombineCounty combineCounties;
    for (const auto& obj : pileOfData)
    {
        obj->accept(combineCounties);
    }

    std::map<string, shared_ptr<psRegionData>> psCounties = combineCounties.getRegionDataPolice();
    std::map<string, shared_ptr<demogRegionData>> demogCounties = combineCounties.getRegionDataDemog();

    // Vote shares
    visitorAggregateLocations aggVoteShares(isValidDistrict, getDistrictParseExpr, [](const regionData& r) {
        return r.getPropertyCount("2020HouseDemCount") /
               (float)(r.getPropertyCount("2020HouseDemCount") + r.getPropertyCount("2020HouseRepCount"));
    });

    for (const auto& obj : theDistrictData)
    {
        obj->accept(aggVoteShares);
    }

    usRegionMap regionMap("DistrictLines");

    regionMap.addParsedMapLayer(
      "DistrictColoring",
      aggVoteShares,
      rwbColorMap,
      [](mapnik::color& c) {
          mapnik::polygon_symbolizer poly_sym;
          put(poly_sym, mapnik::keys::fill, c);
          return (symbolizer)poly_sym;
      },
      "demo/data/cd117");

    mapnik::feature_type_style districtlines_style;
    {
        rule r;
        {
            {
                mapnik::polygon_symbolizer poly_sym;
            }
            mapnik::line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(0, 0, 0));
            put(line_sym, keys::stroke_width, 0.5);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        districtlines_style.add_rule(std::move(r));
    }
    regionMap.addSimpleMapLayer("DistrictLines", "demo/data/cd117", std::move(districtlines_style));

    // State Lines
    mapnik::feature_type_style statelines_style;
    {
        mapnik::rule r;
        {
            mapnik::line_symbolizer line_sym;
            put(line_sym, keys::stroke, color(255, 255, 255));
            put(line_sym, keys::stroke_width, 0.5);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        statelines_style.add_rule(std::move(r));
    }
    regionMap.addSimpleMapLayer("StateLines", "demo/data/cb_2018_us_state_20m", std::move(statelines_style));

    return regionMap.saveToJPG();
    /*[](mapnik::color& c) {
            mapnik::polygon_symbolizer poly_sym;
            put(poly_sym, mapnik::keys::fill, c);
            return (symbolizer)poly_sym;
        }*/
}
