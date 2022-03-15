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

std::vector<std::pair<std::string, mapnik::color>>
  makeParseExpressions(mapnik::Map m,
                       const std::map<std::string, float>& regionValues,
                       const std::vector<mapnik::color> colorMap)
{
    float maxValue = std::max_element(std::begin(regionValues),
                                      std::end(regionValues),
                                      [](const std::pair<std::string, float>& p1,
                                         const std::pair<std::string, float>& p2) { return p1.second < p2.second; })
                       ->second;

    float minValue = std::min_element(std::begin(regionValues),
                                      std::end(regionValues),
                                      [](const std::pair<std::string, float>& p1,
                                         const std::pair<std::string, float>& p2) { return p1.second < p2.second; })
                       ->second;

    std::vector<std::pair<std::string, mapnik::color>> parseExprs;
    for (std::size_t i = 0; i < colorMap.size(); i++)
    {
        parseExprs.push_back({"", colorMap[i]});
    }

    for (auto region : regionValues)
    {
        int index = round(((region.second - minValue) / (maxValue - minValue)) * (colorMap.size() - 1));
        parseExprs[index].first += (parseExprs[index].first.empty() ? +"" : " or ") + region.first;
    }

    return parseExprs;
}

void addStylesToMap(mapnik::Map& m,
                    std::vector<std::pair<std::string, mapnik::color>>& parseExprs,
                    std::vector<std::string>& outLayers,
                    std::string styleName,
                    mapnik::symbolizer (*makeSymbolizer)(mapnik::color&))
{
    int i = 0;
    for (auto parseExprPair : parseExprs)
    {
        // cerr << styleName + std::to_string(i) << ": " << parseExprPair.first << endl;
        if (!parseExprPair.first.empty())
        {
            mapnik::feature_type_style temp_style;
            {
                mapnik::rule temp_rule;
                temp_rule.set_filter(mapnik::parse_expression(parseExprPair.first));
                {
                    mapnik::polygon_symbolizer poly_sym;
                    put(poly_sym, mapnik::keys::fill, parseExprPair.second);
                    temp_rule.append(std::move(makeSymbolizer(parseExprPair.second)));
                }
                temp_style.add_rule(std::move(temp_rule));
            }
            m.insert_style((styleName + std::to_string(i)), std::move(temp_style));
            outLayers.push_back(styleName + std::to_string(i));
            i++;
        }
    }
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

int makeFillOutlineMap(std::string fileName,
                       visitorAggregateLocations& aggFill,
                       visitorAggregateLocations& aggOutline)
{
    using namespace mapnik;

    /*std::vector<color> colorMap = {color(91, 80, 235),
                                   color(95, 245, 155),
                                   color(128, 235, 96),
                                   color(235, 235, 75),
                                   color(245, 213, 91),
                                   color(223, 170, 94),
                                   color(245, 134, 91),
                                   color(235, 91, 101)};*/

    const std::string srs_lcc =
      "+proj=lcc +ellps=GRS80 +lat_0=49 +lon_0=-95 +lat+1=49 +lat_2=77 +datum=NAD83 +units=m +no_defs";
    const std::string srs_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 "
                                 "+units=m +nadgrids=@null +wktext +no_defs +over";

    try
    {
        // std::cout << " creating partial US map... \n";
        datasource_cache::instance().register_datasources("plugins/input/");
        freetype_engine::register_font("/fonts/dejavu-ttf-2.14/DejaVuSans.ttf");

        Map m(1200, 800);
        m.set_background(parse_color("cadetblue"));
        // set projection
        m.set_srs(srs_merc);

        // create styles
        // std::cout << "creating expression rules" << std::endl;
        /*int i = 0;

        // new color map
        std::vector<color> colorMapFill = {color(91, 80, 235),
                                           color(95, 245, 155),
                                           color(128, 235, 96),
                                           color(235, 235, 75),
                                           color(245, 213, 91),
                                           color(223, 170, 94),
                                           color(245, 134, 91),
                                           color(235, 91, 101)};
        std::vector<color> colorMapOutline = {color(0, 0, 0), color(235, 91, 101)};

        std::vector<std::pair<std::string, color>> fillParseExprs =
          makeParseExpressions(m, aggFill.getLocationValues(), colorMapFill);
        std::vector<std::pair<std::string, color>> outlineParseExprs =
          makeParseExpressions(m, aggOutline.getLocationValues(), colorMapOutline);

        // Show state lists
        /*int ind;
        for (auto stateList : popParseExprs)
        {
            cerr << ind++ << ": " << stateList.first << endl;
        }*/

        // DEBUG can remove

        // std::cout << " creating styles ... \n";
        std::vector<std::string> layers;

        /*addStylesToMap(m, fillParseExprs, layers, "fill_styles", [](color& c) {
            mapnik::polygon_symbolizer poly_sym;
            put(poly_sym, mapnik::keys::fill, c);
            return (symbolizer)poly_sym;
        });
        addStylesToMap(m, outlineParseExprs, layers, "outline_styles", [](color& c) {
            line_symbolizer line_sym;
            put(line_sym, keys::stroke, c);
            put(line_sym, keys::stroke_width, 0.5);
            put(line_sym, keys::stroke_linecap, ROUND_CAP);
            put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
            return (symbolizer)line_sym;
        });*/

        // std::cout << " added styles... \n";

        // State (polyline)
        feature_type_style statelines_style;
        {
            rule r;
            {
                line_symbolizer line_sym;
                put(line_sym, keys::stroke, color(255, 255, 255));
                put(line_sym, keys::stroke_width, 0.5);
                put(line_sym, keys::stroke_linecap, ROUND_CAP);
                put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
                r.append(std::move(line_sym));
            }
            statelines_style.add_rule(std::move(r));
        }
        m.insert_style("statelines", std::move(statelines_style));
        
        // District (polyline)
        feature_type_style districtlines_style;
        {
            rule r;
            {
                r.set_filter(mapnik::parse_expression("([STFIPS]='06' and [CDFIPS]='23')"));
                {
                    mapnik::polygon_symbolizer poly_sym;
                }
                line_symbolizer line_sym;
                put(line_sym, keys::stroke, color(0, 0, 0));
                put(line_sym, keys::stroke_width, 0.5);
                put(line_sym, keys::stroke_linecap, ROUND_CAP);
                put(line_sym, keys::stroke_linejoin, ROUND_JOIN);
                r.append(std::move(line_sym));
            }
            districtlines_style.add_rule(std::move(r));
        }
        m.insert_style("districtlines", std::move(districtlines_style));

        // std::cout << " added 2nd style ... \n";

        // Layers
        // county data
        {
            parameters p;
            p["type"] = "shape";
            p["file"] = "demo/data/cb_2018_us_county_20m";
            p["encoding"] = "utf8";

            layer lyr("DataProjMap");
            lyr.set_datasource(datasource_cache::instance().create(p));

            for (auto name : layers)
            {
                // cout << "adding layer: " << name << endl;
                lyr.add_style(name);
            }
            lyr.set_srs(srs_lcc);

            m.add_layer(lyr);
        }
        
        // district boundaries
        {
            parameters p;
            p["type"] = "shape";
            p["file"] = "demo/data/cd117";
            p["encoding"] = "utf8";

            layer lyr("DistrictLines");
            lyr.set_datasource(datasource_cache::instance().create(p));
            lyr.add_style("districtlines");
            lyr.set_srs(srs_lcc);

            m.add_layer(lyr);
        }

        // state boundaries
        {
            parameters p;
            p["type"] = "shape";
            p["file"] = "demo/data/cb_2018_us_state_20m";
            p["encoding"] = "utf8";

            layer lyr("StateLines");
            lyr.set_datasource(datasource_cache::instance().create(p));
            lyr.add_style("statelines");
            lyr.set_srs(srs_lcc);

            m.add_layer(lyr);
        }

        // std::cout << "made layers" << std::endl;

        m.zoom_all();
        m.zoom(0.21);
        m.pan(-950,500);

        image_rgba8 buf(m.width(), m.height());
        agg_renderer<image_rgba8> ren(m, buf);
        ren.apply();
        std::string msg("Writing out data for warm to cool coloring based on police shooting data:\n");
#ifdef HAVE_JPEG
        cout << "Saving " + fileName + " to JPG" << std::endl;
        save_to_file(buf, fileName, "jpeg");
        msg += "- " + fileName + "\n";
#endif
        // save map definition (data + style)
        save_map(m, fileName + ".xml");
    } catch (std::exception const& ex)
    {
        std::cerr << "### std::exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...)
    {
        std::cerr << "### Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    using namespace mapnik;

    // read in a csv file and create a vector of objects representing each counties data
    std::vector<unique_ptr<regionData>> pileOfData = read_csv(
      "demo/gerrymander-stats-tests/county_demographics.csv",
      DEMOG);

    std::vector<unique_ptr<regionData>> thePoliceData =
      read_csv("demo/gerrymander-stats-tests/"
               "fatal-police-shootings-data-Q.csv",
               POLICE);
               
    std::vector<unique_ptr<regionData>> theDistrictData =
      read_csv("demo/gerrymander-stats-tests/"
               "districtInfo.csv",
               DISTRICT);

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

    // AA
    visitorAggregateLocations aggAAPercent(isValidPopCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("Ethnicities.Black Alone", "Population.2020 Population");
    });

    visitorAggregateLocations aggAAFatalGTAAPer(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        std::shared_ptr<regionData> county = combineCounties.getRegionDataDemogData(r.getCounty() + r.getState());
        if (county)
        {
            return ((r.getPropertyPercentage("Ethnicities.Black Alone", "total_incidents") <
                     combineCounties.getRegionDataDemogData(r.getCounty() + r.getState())
                       ->getPropertyPercentage("Ethnicities.Black Alone", "Population.2020 Population"))
                      ? 0.0f
                      : 10.0f);
        }
        else
        {
            cout << "No census data for county: " << r.getCounty() + r.getState() << endl;
            return 0.0f;
        }
    });

    // WHITE
    visitorAggregateLocations aggWPercent(isValidPopCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("Ethnicities.White Alone", "Population.2020 Population");
    });

    visitorAggregateLocations aggWFatalGTWPer(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        std::shared_ptr<regionData> county = combineCounties.getRegionDataDemogData(r.getCounty() + r.getState());
        if (county)
        {
            return ((r.getPropertyPercentage("Ethnicities.White Alone", "total_incidents") <
                     combineCounties.getRegionDataDemogData(r.getCounty() + r.getState())
                       ->getPropertyPercentage("Ethnicities.White Alone", "Population.2020 Population"))
                      ? 0.0f
                      : 10.0f);
        }
        else
        {
            cout << "No census data for county: " << r.getCounty() + r.getState() << endl;
            return 0.0f;
        }
    });

    // LATINX
    visitorAggregateLocations aggLPercent(isValidPopCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("Ethnicities.Hispanic or Latino", "Population.2020 Population");
    });

    visitorAggregateLocations aggLFatalGTLPer(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        std::shared_ptr<regionData> county = combineCounties.getRegionDataDemogData(r.getCounty() + r.getState());
        if (county)
        {
            return ((r.getPropertyPercentage("Ethnicities.Hispanic or Latino", "total_incidents") <
                     combineCounties.getRegionDataDemogData(r.getCounty() + r.getState())
                       ->getPropertyPercentage("Ethnicities.Hispanic or Latino", "Population.2020 Population"))
                      ? 0.0f
                      : 10.0f);
        }
        else
        {
            cout << "No census data for county: " << r.getCounty() + r.getState() << endl;
            return 0.0f;
        }
    });

    // ASIAN AMERICAN
    visitorAggregateLocations aggASPercent(isValidPopCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("Ethnicities.Asian Alone", "Population.2020 Population");
    });

    visitorAggregateLocations aggASFatalGTASPer(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        std::shared_ptr<regionData> county = combineCounties.getRegionDataDemogData(r.getCounty() + r.getState());
        if (county)
        {
            return ((r.getPropertyPercentage("Ethnicities.Asian Alone", "total_incidents") <
                     combineCounties.getRegionDataDemogData(r.getCounty() + r.getState())
                       ->getPropertyPercentage("Ethnicities.Asian Alone", "Population.2020 Population"))
                      ? 0.0f
                      : 10.0f);
        }
        else
        {
            cout << "No census data for county: " << r.getCounty() + r.getState() << endl;
            return 0.0f;
        }
    });

    // NATIVE AMERICAN
    visitorAggregateLocations aggNAPercent(isValidPopCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("Ethnicities.American Indian and Alaska Native Alone",
                                              "Population.2020 Population");
    });

    visitorAggregateLocations aggNAFatalGTNAPer(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        std::shared_ptr<regionData> county = combineCounties.getRegionDataDemogData(r.getCounty() + r.getState());
        if (county)
        {
            return ((r.getPropertyPercentage("Ethnicities.American Indian and Alaska Native Alone", "total_incidents") <
                     combineCounties.getRegionDataDemogData(r.getCounty() + r.getState())
                       ->getPropertyPercentage("Ethnicities.American Indian and Alaska Native Alone",
                                               "Population.2020 Population"))
                      ? 0.0f
                      : 10.0f);
        }
        else
        {
            cout << "No census data for county: " << r.getCounty() + r.getState() << endl;
            return 0.0f;
        }
    });

    // HS vs MI
    visitorAggregateLocations aggHSPercent(isValidPopCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("Education.High School or Higher", "Population.2020 Population");
    });

    visitorAggregateLocations aggMIGT50(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        return ((r.getPropertyPercentage("signs_of_mental_illness", "total_incidents") < 50) ? 0.0f : 10.0f);
    });

    // Armed vs Body Camera
    visitorAggregateLocations aggArmedPercent(isValidIncidentCounty, getRegionFips, [](const regionData& r) {
        return (float)r.getPropertyPercentage("armed", "total_incidents");
    });

    visitorAggregateLocations aggBodyCamPercent(isValidIncidentCounty, getRegionFips, [&](const regionData& r) {
        return ((r.getPropertyPercentage("body_camera", "total_incidents") < 50) ? 0.0f : 10.0f);
    });

    for (const auto& obj : demogCounties)
    {
        obj.second->accept(aggAAPercent);
        obj.second->accept(aggWPercent);
        obj.second->accept(aggLPercent);
        obj.second->accept(aggASPercent);
        obj.second->accept(aggNAPercent);
        obj.second->accept(aggHSPercent);
    }
    for (const auto& obj : psCounties)
    {
        obj.second->accept(aggAAFatalGTAAPer);
        obj.second->accept(aggWFatalGTWPer);
        obj.second->accept(aggLFatalGTLPer);
        obj.second->accept(aggASFatalGTASPer);
        obj.second->accept(aggNAFatalGTNAPer);
        obj.second->accept(aggMIGT50);
        obj.second->accept(aggArmedPercent);
        obj.second->accept(aggBodyCamPercent);
    }

    //makeFillOutlineMap("AAvsPS.jpg", aggAAPercent, aggAAFatalGTAAPer);
    //makeFillOutlineMap("WvsPS.jpg", aggWPercent, aggWFatalGTWPer);
    //makeFillOutlineMap("LvsPS.jpg", aggLPercent, aggLFatalGTLPer);
    //makeFillOutlineMap("ASvsPS.jpg", aggASPercent, aggASFatalGTASPer);
    //makeFillOutlineMap("NAvsPS.jpg", aggNAPercent, aggNAFatalGTNAPer);
    //makeFillOutlineMap("HSvsMI.jpg", aggHSPercent, aggMIGT50);
    //makeFillOutlineMap("ArmedvsBodyCam.jpg", aggArmedPercent, aggBodyCamPercent);
    makeFillOutlineMap("DistrictLines.jpg", aggArmedPercent, aggBodyCamPercent);
}
