#include "GeocacheFetcher.h"
#include "jsmn.h"

#define COUNTOF(x) \
   ( sizeof(x) / sizeof((x)[0]) )

const char *GeocacheFetcher::host = "www.opencaching.us";
// Documentation for the search API:
// http://www.opencaching.us/okapi/services/caches/search/nearest.html
// Documentation for the retrieval API:
// http://www.opencaching.us/okapi/services/caches/geocaches.html
// Documentation for the combined search and retrieve API:
// http://www.opencaching.us/okapi/services/caches/shortcuts/search_and_retrieve.html
const char *GeocacheFetcher::searchNearestPath =
  "/okapi/services/caches/shortcuts/search_and_retrieve?"
  "search_method=services/caches/search/nearest&"
  "consumer_key=%s&"
  "search_params={\"name\":\"*%s*\",\"center\":\"%f|%f\",\"limit\":1,\"offset\":%d}&"
  "retr_method=services/caches/geocaches&"
  "retr_params={}&"
  "wrap=true";
  
GeocacheFetcher::GeocacheFetcher(const char *opencacheApiKey)
: apiKey(opencacheApiKey),
  headers{
    { NULL, NULL }
  }
{
}

Point GeocacheFetcher::searchNearest(const char *query, double latitude, double longitude, unsigned int offset) {
  if(fetchNearest(query, latitude, longitude, offset) == 200) {
    return parseNearest();
  } else {
    return Point();
  }
}

int GeocacheFetcher::fetchNearest(const char *query, double latitude, double longitude, unsigned int offset) {
  char path[400];

  snprintf(path, sizeof(path), searchNearestPath, apiKey, query, latitude, longitude, offset);

  request.hostname = host;
  request.port = 80;
  request.path = path;
  http.get(request, response, headers);

  return response.status;
}

Point GeocacheFetcher::parseNearest() {
  jsmn_parser parser;
  jsmntok_t tokens[20];
  int r;
  String &body = response.body;
  const char *bodyStr = body.c_str();

  // Parse JSON into tokens
  jsmn_init(&parser, NULL);

  r = jsmn_parse(&parser, body, body.length(), tokens, COUNTOF(tokens), NULL);

  Point result;
  // find the "location" JSON key
  for(int i = 0; i < r; i++) {
    jsmntok_t tok = tokens[i];
    const char *tokenStr = &bodyStr[tok.start];
    if(strncmp(tokenStr, "location", strlen("location")) == 0) {
      jsmntok_t locationTok = tokens[i + 1];

      // Extract the location as 12.47489|-84.45445
      int startPos = locationTok.start;
      int sepPos = body.indexOf("|", startPos);
      int endPos = locationTok.end;

      result.latitude = body.substring(startPos, sepPos - 1).toFloat();
      result.longitude = body.substring(sepPos + 1, endPos).toFloat();
      break;
    }
  }
  return result;
}
