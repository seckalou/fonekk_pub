project(mapshot_tool)

include_directories(
  .
  ${OMIM_ROOT}/3party/glm
  ${OMIM_ROOT}/3party/gflags/src
)

set(
  SRC
  mapshot.cpp
)

omim_add_executable(${PROJECT_NAME} ${SRC})

omim_link_libraries(
  ${PROJECT_NAME}
  map
  software_renderer
  drape_frontend
  shaders
  routing
  search
  storage
  tracking
  traffic
  routing_common
  ugc
  drape
  partners_api
  local_ads
  kml
  indexer
  platform
  editor
  geometry
  coding
  base
  freetype
  expat
  gflags
  icu
  agg
  jansson
  protobuf
  osrm
  stats_client
  minizip
  succinct
  pugixml
  oauthcpp
  opening_hours
  stb_image
  sdf_image
  vulkan_wrapper
  ${LIBZ}
)

if (PLATFORM_LINUX)
  omim_link_libraries(
    ${PROJECT_NAME}
    dl
  )
endif()

link_opengl(${PROJECT_NAME})
link_qt5_core(${PROJECT_NAME})
