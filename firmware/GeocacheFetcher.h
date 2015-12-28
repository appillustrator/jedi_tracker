#include "HttpClient.h"
#include "GeoPoint.h"

class GeocacheFetcher {
  public:
  GeocacheFetcher(const char *opencacheApiKey);

  Point searchNearest(const char *query, double latitude, double longitude, unsigned int offset = 0);

  private:

  int fetchNearest(const char *query, double latitude, double longitude, unsigned int offset = 0);
  Point parseNearest();
  static const char *host;
  static const char *searchNearestPath;

  const char *apiKey;

  HttpClient http;

  http_header_t headers[1];

  http_request_t request;
  http_response_t response;
};
