#pragma once

#include "std/vector.hpp"
#include "std/string.hpp"


namespace downloader
{
  class HttpRequest;

  void GetDefaultServerList(vector<string> & urls);
  void GetServerListFromRequest(HttpRequest const & request, vector<string> & urls);
}
