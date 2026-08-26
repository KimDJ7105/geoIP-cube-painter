#ifndef GEOIP_MANAGER_H
#define GEOIP_MANAGER_H

#include <string>
#include <string_view>
#include <maxminddb.h>

class GeoIPManager {
public:
    GeoIPManager();
    ~GeoIPManager();

    // 데이터베이스 파일을 열어 초기화하는 함수
    bool initialize(const std::string& db_path);

    // 내부 자원을 명시적으로 해제하는 함수
    void close();

    // IP 주소를 받아 국가 코드를 반환하는 핵심 함수
    std::string get_country_code(std::string_view ip_address);

private:
    MMDB_s mmdb_;
    bool is_initialized_;
};

#endif // GEOIP_MANAGER_H