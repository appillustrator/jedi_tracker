#pragma once

struct Point {
  double latitude;
  double longitude;

  Point() : latitude(0), longitude(0)
  { }

  bool isValid() {
    return latitude != 0 && longitude != 0;
  }
};
