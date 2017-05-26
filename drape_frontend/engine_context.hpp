#pragma once

#include "drape_frontend/custom_symbol.hpp"
#include "drape_frontend/map_shape.hpp"
#include "drape_frontend/tile_utils.hpp"
#include "drape_frontend/threads_commutator.hpp"
#include "drape_frontend/traffic_generator.hpp"

#include "drape/constants.hpp"
#include "drape/pointers.hpp"

namespace dp
{
class TextureManager;
} // namespace dp

namespace df
{
class Message;

class EngineContext
{
public:
  EngineContext(TileKey tileKey,
                ref_ptr<ThreadsCommutator> commutator,
                ref_ptr<dp::TextureManager> texMng,
                CustomSymbolsContextWeakPtr customSymbolsContext,
                bool is3dBuildingsEnabled,
                bool isTrafficEnabled,
                int displacementMode);

  TileKey const & GetTileKey() const { return m_tileKey; }
  bool Is3dBuildingsEnabled() const { return m_3dBuildingsEnabled; }
  bool IsTrafficEnabled() const { return m_trafficEnabled; }
  int GetDisplacementMode() const { return m_displacementMode; }
  CustomSymbolsContextWeakPtr GetCustomSymbolsContext() const { return m_customSymbolsContext; }
  ref_ptr<dp::TextureManager> GetTextureManager() const;

  void BeginReadTile();
  void Flush(TMapShapes && shapes);
  void FlushOverlays(TMapShapes && shapes);
  void FlushTrafficGeometry(TrafficSegmentsGeometry && geometry);
  void EndReadTile();

private:
  void PostMessage(drape_ptr<Message> && message);

  TileKey m_tileKey;
  ref_ptr<ThreadsCommutator> m_commutator;
  ref_ptr<dp::TextureManager> m_texMng;
  CustomSymbolsContextWeakPtr m_customSymbolsContext;
  bool m_3dBuildingsEnabled;
  bool m_trafficEnabled;
  int m_displacementMode;
};
}  // namespace df
