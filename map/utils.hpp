#pragma once

#include "metrics/eye_info.hpp"

#include <boost/optional.hpp>

#include <string>

namespace place_page
{
class Info;
}

class FeatureType;

namespace utils
{
eye::MapObject MakeEyeMapObject(place_page::Info const & info);
eye::MapObject MakeEyeMapObject(FeatureType & ft);

void RegisterEyeEventIfPossible(eye::MapObject::Event::Type const type,
                                boost::optional<m2::PointD> const & userPos,
                                place_page::Info const & info);

std::string Nameify(std::string name);

}  // namespace utils
