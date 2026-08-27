#ifndef COUNTRY_CENTROIDS_H
#define COUNTRY_CENTROIDS_H

#include <string>
#include <utility>

// 국가 코드(ISO 2자리)로 시작 좌표(맵 픽셀 좌표, x/y)를 조회합니다.
// 매핑에 없는 국가는 지도 정중앙(적도-본초자오선 투영 위치)으로 폴백합니다.
// 반환되는 국가 코드는 항상 2자리로 정규화됩니다 (매핑에 없거나 LOCAL/UNKNOWN 등은 "XX").
std::pair<double, double> get_start_position(const std::string& raw_country_code);

// 게임에서 사용할 2자리 국가 코드로 정규화.
// 이미 2자리 코드면 그대로 사용. LOCAL/UNKNOWN 등 유효하지 않은 경우(로컬 개발 환경 등)에는
// 매번 같은 위치로 몰리지 않도록 대표 좌표 테이블에서 무작위로 한 국가를 골라 대신 사용한다.
std::string normalize_country_code(const std::string& raw_country_code);

#endif // COUNTRY_CENTROIDS_H
