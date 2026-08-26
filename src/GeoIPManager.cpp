#include "GeoIPManager.h"
#include <iostream>

GeoIPManager::GeoIPManager() : is_initialized_(false) {}

GeoIPManager::~GeoIPManager() {
    close();
}

bool GeoIPManager::initialize(const std::string& db_path) {
    if (is_initialized_) {
        return true;
    }

    int status = MMDB_open(db_path.c_str(), MMDB_MODE_MMAP, &mmdb_);
    if (status != MMDB_SUCCESS) {
        std::cerr << "MMDB 오픈 실패: " << MMDB_strerror(status) << std::endl;
        return false;
    }

    is_initialized_ = true;
    return true;
}

void GeoIPManager::close() {
    if (is_initialized_) {
        MMDB_close(&mmdb_);
        is_initialized_ = false;
        std::cout << "GeoIP 데이터베이스 자원 해제 완료." << std::endl;
    }
}

std::string GeoIPManager::get_country_code(std::string_view ip_address) {
    if (!is_initialized_) {
        return "UNKNOWN";
    }

    int gai_error = 0;
    int mmdb_error = 0;

    // string_view를 C 스타일 문자열로 안전하게 변환하여 조회
    MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb_, std::string(ip_address).c_str(), &gai_error, &mmdb_error);

    if (gai_error != 0 || mmdb_error != MMDB_SUCCESS || !result.found_entry) {
        return "UNKNOWN";
    }

    MMDB_entry_data_s entry_data;
    int lookup_status = MMDB_get_value(&result.entry, &entry_data, "country", "iso_code", NULL);

    if (lookup_status == MMDB_SUCCESS && entry_data.has_data) {
        return std::string(entry_data.utf8_string, entry_data.data_size);
    }

    return "UNKNOWN";
}