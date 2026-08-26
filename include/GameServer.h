#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <uwebsockets/App.h>
#include "PerSocketData.h"

// GeoIPManager 클래스의 전방 선언 (상호 참조 의존성 최소화)
class GeoIPManager;

class GameServer {
public:
    GameServer(GeoIPManager& geoip_manager);
    ~GameServer();

    // 지정된 포트로 웹소켓 서버를 가동하고 루프를 시작하는 함수
    bool start(int port);

private:
    // 약 33ms 주기 (초당 30프레임/30Hz 틱레이트) 설정
    // 만약 초당 60프레임을 원하신다면 16
    static constexpr int SERVER_TICK_RATE = 33;
    static constexpr double PLAYER_MOVE_SPEED = 5.0;
    // uWebSockets의 실제 Behavior 세부 구성을 세팅하는 헬퍼 함수
    void setup_behavior();
    void setup_game_loop(); // 고정 주기로 실행될 게임 루프 설정 함수 추가 

    uWS::App::WebSocketBehavior<PerSocketData> behavior_;
    GeoIPManager& geoip_manager_; // 외부에서 주입받은 GeoIP 모듈 참조자
    std::atomic<uint32_t> next_user_id_{1};

    // 현재 서버에 접속 중인 모든 웹소켓 세션의 주소들을 관리하는 셋
    // 멀티 스레드 변경시 주의 필요. 
    std::unordered_set<uWS::WebSocket<false, true, PerSocketData>*> clients_;
};

#endif // GAME_SERVER_H