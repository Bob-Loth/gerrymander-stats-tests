#include "usRegionMap.h"

usRegionMap::usRegionMap(std::string mapName)
    : name(mapName)
    , map(2400, 1600)
{
    srs_lcc = "+proj=lcc +ellps=GRS80 +lat_0=49 +lon_0=-95 +lat+1=49 +lat_2=77 +datum=NAD83 +units=m +no_defs";
    const std::string srs_merc = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 "
                                 "+units=m +nadgrids=@null +wktext +no_defs +over";

    // std::cout << " creating partial US map... \n";
    mapnik::datasource_cache::instance().register_datasources("plugins/input/");
    mapnik::freetype_engine::register_font("/fonts/dejavu-ttf-2.14/DejaVuSans.ttf");

    map.set_background(mapnik::color(235, 233, 221));
    // set projection
    map.set_srs(srs_merc);
}

std::vector<std::pair<std::string, mapnik::color>>
  usRegionMap::makeParseExpressions(mapnik::Map m,
                                    const std::map<std::string, float>& regionValues,
                                    const std::vector<mapnik::color>& colorMap)
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
        //cout << index << " : " << region.second << " : " << minValue << " : " << maxValue << " : " << region.first << endl;
        parseExprs[index].first += (parseExprs[index].first.empty() ? +"" : " or ") + region.first;
    }

    return parseExprs;
}

void usRegionMap::addStylesToMap(mapnik::Map& m,
                                 std::vector<std::pair<std::string, mapnik::color>>& parseExprs,
                                 std::vector<std::string>& outLayers,
                                 std::string styleName,
                                 mapnik::symbolizer (*makeSymbolizer)(mapnik::color&))
{
    int i = 0;
    for (auto parseExprPair : parseExprs)
    {
        //cerr << styleName + std::to_string(i) << ": " << parseExprPair.first << endl;
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

void usRegionMap::addParsedMapLayer(std::string layerName,
                                    visitorAggregateLocations& agg,
                                    std::vector<mapnik::color>& colors,
                                    mapnik::symbolizer (*makeSymbolizer)(mapnik::color&),
                                    std::string shapeFile)
{
    std::vector<std::pair<std::string, mapnik::color>> fillParseExprs =
      makeParseExpressions(map, agg.getLocationValues(), colors);

    std::vector<std::string> layers;

    addStylesToMap(map, fillParseExprs, layers, layerName + "_styles", makeSymbolizer);

    {
        mapnik::parameters p;
        p["type"] = "shape";
        p["file"] = shapeFile;
        p["encoding"] = "utf8";

        mapnik::layer lyr(layerName);
        lyr.set_datasource(mapnik::datasource_cache::instance().create(p));
        lyr.set_srs(srs_lcc);

        for (auto name : layers)
        {
            // cout << "adding layer: " << name << endl;
            lyr.add_style(name);
        }
        map.add_layer(lyr);
    }
    cout << "added layer\n";
    for (auto l : map.layers())
    {
        cout << l.name() << endl;
    }
}

void usRegionMap::addSimpleMapLayer(std::string layerName,
                                    std::string shapeFile,
                                    mapnik::feature_type_style featureTypeStyle)
{
    mapnik::feature_type_style districtlines_style;
    {
        mapnik::rule r;
        {
            {
                mapnik::polygon_symbolizer poly_sym;
            }
            mapnik::line_symbolizer line_sym;
            put(line_sym, mapnik::keys::stroke, mapnik::color(0, 0, 0));
            put(line_sym, mapnik::keys::stroke_width, 0.5);
            put(line_sym, mapnik::keys::stroke_linecap, mapnik::ROUND_CAP);
            put(line_sym, mapnik::keys::stroke_linejoin, mapnik::ROUND_JOIN);
            r.append(std::move(line_sym));
        }
        districtlines_style.add_rule(std::move(r));
    }
    map.insert_style(layerName + "_style", std::move(districtlines_style));
    {
        mapnik::parameters p;
        p["type"] = "shape";
        p["file"] = shapeFile;
        p["encoding"] = "utf8";

        mapnik::layer lyr(layerName);
        lyr.set_datasource(mapnik::datasource_cache::instance().create(p));
        lyr.add_style(layerName + "_style");
        lyr.set_srs(srs_lcc);

        map.add_layer(lyr);
    }
}

int usRegionMap::saveToJPG()
{
    try
    {
        string fileName = name + ".jpg";
        map.zoom_all();
        map.zoom(0.21);
        map.pan(-1900, 1000);

        mapnik::image_rgba8 buf(map.width(), map.height());
        mapnik::agg_renderer<mapnik::image_rgba8> ren(map, buf);
        ren.apply();
        std::string msg("Writing out data for warm to cool coloring based on police shooting data:\n");
#ifdef HAVE_JPEG
        cout << "Saving " + fileName + " to JPG" << std::endl;
        save_to_file(buf, fileName, "jpeg");
        msg += "- " + fileName + "\n";
#endif
        // save map definition (data + style)
        mapnik::save_map(map, fileName + ".xml");
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