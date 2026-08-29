#include "CountryCentroids.h"
#include "MapConfig.h"
#include <unordered_map>
#include <random>
#include <iterator>
#include <cstdlib>

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

namespace {

// 데모/시연 목적: GEOIP_DEMO_RANDOM_COUNTRY 환경변수가 설정돼 있으면, 실제 GeoIP 판정 결과와
// 무관하게 항상 무작위 국가를 배정한다. 같은 공인 IP(로컬망 등)에서 여러 명이 접속해도
// 다양한 국가/색으로 보이게 하기 위한 시연용 오버라이드 — 기본값은 꺼짐(실제 GeoIP 그대로 사용).
bool demo_random_country_enabled() {
    static const bool enabled = (std::getenv("GEOIP_DEMO_RANDOM_COUNTRY") != nullptr);
    return enabled;
}

}  // namespace

std::string normalize_country_code(const std::string& raw_country_code) {
    if (!demo_random_country_enabled() && raw_country_code.size() == 2) return raw_country_code;

    // 데모 모드이거나, LOCAL/UNKNOWN 등 2자리 코드가 아닌 경우 — 매번 같은 자리로 몰리지 않도록
    // 대표 좌표 테이블에서 무작위로 한 국가를 골라 대신 사용한다.
    const auto& table = centroid_table();
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<size_t> dist(0, table.size() - 1);

    auto it = table.begin();
    std::advance(it, dist(rng));
    return it->first;
}

std::pair<double, double> get_start_position(const std::string& normalized_country_code) {
    // 호출부(GameServer.cpp)가 이미 normalize_country_code()를 거친 코드를 넘겨준다는 전제.
    // 여기서 다시 normalize_country_code()를 호출하면 데모 모드에서 매번 새로 무작위 추첨해
    // user_data->country에 저장된 코드(라벨/칠하는 색)와 실제 스폰 위치가 서로 다른 국가로
    // 어긋나는 버그가 있었음 — 위치는 이 코드를 그대로 신뢰해서 조회한다.
    const std::string& code = normalized_country_code;

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
