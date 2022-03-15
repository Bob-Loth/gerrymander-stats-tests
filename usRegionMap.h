#ifndef US_REGION_MAP_H
#define US_REGION_MAP_H

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
#include <vector>

#include "regionData.h"
#include "visitorAggregateLocations.h"

class usRegionMap
{
  public:
    usRegionMap(std::string mapName);

    void addParsedMapLayer(std::string layerName,
                           visitorAggregateLocations& agg,
                           std::vector<mapnik::color>& colors,
                           mapnik::symbolizer (*makeSymbolizer)(mapnik::color&),
                           std::string shapeFile);

    void addSimpleMapLayer(std::string layerName, std::string shapeFile, mapnik::feature_type_style featureTypeStyle);
    // Save map to <mapName>.JPG
    // Return: EXIT_SUCCESS or EXIT_FAILURE
    int saveToJPG();

  private:
    mapnik::Map map;
    std::string name;
    std::string srs_lcc;

    std::vector<std::pair<std::string, mapnik::color>>
      makeParseExpressions(mapnik::Map m,
                           const std::map<std::string, float>& regionValues,
                           const std::vector<mapnik::color>& colorMap);

    void addStylesToMap(mapnik::Map& m,
                    std::vector<std::pair<std::string, mapnik::color>>& parseExprs,
                    std::vector<std::string>& outLayers,
                    std::string styleName,
                    mapnik::symbolizer (*makeSymbolizer)(mapnik::color&));
};

#endif