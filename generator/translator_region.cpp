#include "generator/translator_region.hpp"

#include "generator/emitter_interface.hpp"
#include "generator/feature_builder.hpp"
#include "generator/generate_info.hpp"
#include "generator/holes.hpp"
#include "generator/intermediate_data.hpp"
#include "generator/osm2type.hpp"
#include "generator/osm_element.hpp"
#include "generator/regions/region_info_collector.hpp"

#include "indexer/classificator.hpp"

#include "geometry/point2d.hpp"

#include "base/assert.hpp"

#include <set>
#include <string>
#include <vector>

namespace generator
{
TranslatorRegion::TranslatorRegion(std::shared_ptr<EmitterInterface> emitter,
                                   cache::IntermediateDataReader & holder,
                                   regions::RegionInfoCollector & regionInfoCollector)
  : m_emitter(emitter),
    m_holder(holder),
    m_regionInfoCollector(regionInfoCollector)
{
}

void TranslatorRegion::EmitElement(OsmElement * p)
{
  CHECK(p, ("Tried to emit a null OsmElement"));

  FeatureParams params;
  if (!(IsSuitableElement(p) && ParseParams(p, params)))
    return;

  switch (p->type)
  {
  case OsmElement::EntityType::Node:
    BuildFeatureAndEmitFromNode(p, params);
    break;
  case OsmElement::EntityType::Relation:
    BuildFeatureAndEmitFromRelation(p, params);
    break;
  case OsmElement::EntityType::Way:
    BuildFeatureAndEmitFromWay(p, params);
    break;
  default:
    break;
  }
}

bool TranslatorRegion::IsSuitableElement(OsmElement const * p) const
{
  static std::set<std::string> const places = {"city", "town", "village", "suburb", "neighbourhood",
                                               "hamlet", "locality", "isolated_dwelling"};

  for (auto const & t : p->Tags())
  {
    if (t.key == "place" && places.find(t.value) != places.end())
      return true;

    auto const & members = p->Members();
    auto const pred = [](OsmElement::Member const & m) { return m.role == "admin_centre"; };
    if (t.key == "boundary" && t.value == "political" &&
        std::find_if(std::begin(members), std::end(members), pred) != std::end(members))
      return true;

    if (t.key == "boundary" && t.value == "administrative")
      return true;
  }

  return false;
}

void TranslatorRegion::AddInfoAboutRegion(OsmElement const * p, base::GeoObjectId osmId) const
{
  m_regionInfoCollector.Add(osmId, *p);
}

bool TranslatorRegion::ParseParams(OsmElement * p, FeatureParams & params) const
{
  ftype::GetNameAndType(p, params, [] (uint32_t type) {
    return classif().IsTypeValid(type);
  });
  return params.IsValid();
}

void TranslatorRegion::BuildFeatureAndEmitFromRelation(OsmElement const * p, FeatureParams & params)
{
  HolesRelation helper(m_holder);
  helper.Build(p);
  auto const & holesGeometry = helper.GetHoles();
  auto & outer = helper.GetOuter();
  outer.ForEachArea(true, [&] (FeatureBuilder1::PointSeq const & pts,
                    std::vector<uint64_t> const & ids)
  {
    FeatureBuilder1 fb;
    for (uint64_t id : ids)
      fb.AddOsmId(base::MakeOsmWay(id));

    for (auto const & pt : pts)
      fb.AddPoint(pt);

    auto const id = base::MakeOsmRelation(p->id);
    fb.AddOsmId(id);
    if (!fb.IsGeometryClosed())
      return;

    fb.SetAreaAddHoles(holesGeometry);
    fb.SetParams(params);
    AddInfoAboutRegion(p, id);
    (*m_emitter)(fb);
  });
}

void TranslatorRegion::BuildFeatureAndEmitFromWay(OsmElement const * p, FeatureParams & params)
{
  FeatureBuilder1 fb;
  m2::PointD pt;
  for (uint64_t ref : p->Nodes())
  {
    if (!m_holder.GetNode(ref, pt.y, pt.x))
      return;

    fb.AddPoint(pt);
  }

  auto const id = base::MakeOsmWay(p->id);
  fb.SetOsmId(id);
  fb.SetParams(params);
  if (!fb.IsGeometryClosed())
    return;

  fb.SetArea();
  AddInfoAboutRegion(p, id);
  (*m_emitter)(fb);
}

void TranslatorRegion::BuildFeatureAndEmitFromNode(OsmElement const * p, FeatureParams & params)
{
  m2::PointD const pt = MercatorBounds::FromLatLon(p->lat, p->lon);
  FeatureBuilder1 fb;
  fb.SetCenter(pt);
  auto const id = base::MakeOsmNode(p->id);
  fb.SetOsmId(id);
  fb.SetParams(params);
  AddInfoAboutRegion(p, id);
  (*m_emitter)(fb);
}
}  // namespace generator
