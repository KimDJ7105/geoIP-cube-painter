#include "CountryCentroids.h"
#include "MapConfig.h"
#include <unordered_map>

namespace {

// 주요 국가의 대표 좌표 (위도, 경도). 지리적 중심이 아니라 "그 나라 하면 떠오르는" 대표 지점 위주.
const std::unordered_map<std::string, std::pair<double, double>>& centroid_table() {
    static const std::unordered_map<std::string, std::pair<double, double>> table = {
        {"KR", {37.5, 127.0}},   {"JP", {36.0, 138.0}},   {"CN", {35.0, 105.0}},
        {"TW", {23.7, 121.0}},   {"HK", {22.3, 114.2}},   {"MN", {46.8, 103.8}},
        {"US", {39.8, -98.5}},   {"CA", {56.1, -106.3}},  {"MX", {23.6, -102.5}},
        {"BR", {-14.2, -51.9}},  {"AR", {-38.4, -63.6}},  {"CL", {-35.6, -71.5}},
        {"CO", {4.5, -74.2}},    {"PE", {-9.1, -75.0}},
        {"GB", {54.0, -2.0}},    {"IE", {53.4, -8.2}},    {"FR", {46.6, 2.2}},
        {"DE", {51.1, 10.4}},    {"NL", {52.1, 5.3}},     {"BE", {50.5, 4.5}},
        {"ES", {40.4, -3.7}},    {"PT", {39.4, -8.2}},    {"IT", {41.9, 12.5}},
        {"CH", {46.8, 8.2}},     {"AT", {47.5, 14.5}},    {"SE", {60.1, 18.6}},
        {"NO", {60.5, 8.5}},     {"FI", {61.9, 25.7}},    {"DK", {56.3, 9.5}},
        {"PL", {51.9, 19.1}},    {"CZ", {49.8, 15.5}},    {"HU", {47.2, 19.5}},
        {"RO", {45.9, 24.9}},    {"GR", {39.1, 21.8}},    {"UA", {48.4, 31.2}},
        {"RU", {61.5, 105.3}},   {"TR", {38.9, 35.2}},
        {"IN", {20.6, 79.0}},    {"PK", {30.4, 69.3}},    {"BD", {23.7, 90.4}},
        {"ID", {-0.8, 113.9}},   {"PH", {12.9, 121.8}},   {"VN", {14.1, 108.3}},
        {"TH", {15.9, 100.9}},   {"MY", {4.2, 101.9}},    {"SG", {1.35, 103.8}},
        {"AU", {-25.3, 133.8}},  {"NZ", {-41.0, 174.9}},
        {"EG", {26.8, 30.8}},    {"ZA", {-30.6, 22.9}},   {"NG", {9.1, 8.7}},
        {"SA", {23.9, 45.1}},    {"AE", {23.4, 53.8}},    {"IL", {31.0, 34.8}},
    };
    return table;
}

double clampd(double v, double lo, double hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

}  // namespace

std::string normalize_country_code(const std::string& raw_country_code) {
    if (raw_country_code.size() != 2) return "XX";
    return raw_country_code;
}

std::pair<double, double> get_start_position(const std::string& raw_country_code) {
    const std::string code = normalize_country_code(raw_country_code);

    double lat = 0.0;   // 폴백: 적도
    double lon = 0.0;   // 폴백: 본초자오선 (=지도 정중앙)

    const auto& table = centroid_table();
    auto it = table.find(code);
    if (it != table.end()) {
        lat = it->second.first;
        lon = it->second.second;
    }

    // 등장방형 투영: lon[-180,180] -> x[0,MAP_WIDTH], lat[90,-90] -> y[0,MAP_HEIGHT]
    double x = (lon + 180.0) / 360.0 * MAP_WIDTH;
    double y = (90.0 - lat) / 180.0 * MAP_HEIGHT;

    x = clampd(x, 0.0, MAP_WIDTH - 1.0);
    y = clampd(y, 0.0, MAP_HEIGHT - 1.0);

    return {x, y};
}
