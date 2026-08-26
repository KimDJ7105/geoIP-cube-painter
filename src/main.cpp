#include "GeoIPManager.h"
#include "GameServer.h"
#include <iostream>

int main() {
    // 1. GeoIP 매니저 생성 및 데이터베이스 초기화
    GeoIPManager geoip_manager;
    std::string db_path = "../db/GeoLite2-Country.mmdb";
    
    if (!geoip_manager.initialize(db_path)) {
        std::cerr << "서버 초기화 실패: GeoIP 데이터베이스를 로드할 수 없습니다." << std::endl;
        return 1;
    }

    // 2. 게임 서버 생성 (의존성 주입: GeoIP 매니저의 참조자를 넘겨줍니다)
    GameServer server(geoip_manager);

    // 3. 포트 3000으로 서버 가동 및 루프 진입
    int port = 7777;
    server.start(port);

    // 4. 서버 다운 시 자원 정리 (정상 종료 플로우)
    geoip_manager.close();
    return 0;
}