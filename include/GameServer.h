#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <uwebsockets/App.h>
#include <array>
#include "PerSocketData.h"
#include "MapConfig.h"

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
    static constexpr double PLAYER_MOVE_SPEED = 120.0;  // 픽셀/초 (월드 좌표 = 캔버스 픽셀 1:1)
    // uWebSockets의 실제 Behavior 세부 구성을 세팅하는 헬퍼 함수
    void setup_behavior();
    void setup_game_loop(); // 고정 주기로 실행될 게임 루프 설정 함수 추가

    // 유저가 새 칸으로 진입했을 때 그 칸의 소유권을 갱신.
    // AOI(관심 영역) 안에 있는 클라이언트에게는 즉시 개별 전송.
    // 칠해질 때마다 전역 dirty_cells_에도 기록해 두어, 주기적 앰비언트 동기화가
    // 그리드 전체를 스캔하지 않고도 "이번 주기에 뭐가 바뀌었는지" 바로 알 수 있게 함.
    void try_paint_cell(uWS::WebSocket<false, true, PerSocketData>* ws, PerSocketData* user_data);

    // 신규 접속자 또는 MAP_REQUEST 요청 시, 현재까지 칠해진 칸 전체를 벌크 패킷 하나로 전송
    // (정확성이 중요한 온디맨드 요청이라 델타가 아니라 항상 전체 스냅샷)
    void send_grid_snapshot(uWS::WebSocket<false, true, PerSocketData>* ws);

    // 신규 접속자에게 이미 접속해 있는 다른 유저들의 현재 위치를 한 번에 전송
    // (없으면 상대가 다음에 움직이기 전까지 화면에 보이지 않는 문제 발생)
    void send_player_snapshot(uWS::WebSocket<false, true, PerSocketData>* ws);

    // 특정 칸이 어떤 유저의 AOI(뷰포트 + 여유 마진) 안에 있는지 판정
    bool is_near_viewer(double viewer_x, double viewer_y, int cell_x, int cell_y) const;

    // dirty_cells_에 쌓인 칸들만 벌크 패킷으로 묶어 전체 접속자에게 한 번에 publish하고 비움
    // (유저별 상태가 아니라 전역 상태라 그리드 스캔도, 유저별 반복도 필요 없음)
    void flush_dirty_cells();

    uWS::App::WebSocketBehavior<PerSocketData> behavior_;
    GeoIPManager& geoip_manager_; // 외부에서 주입받은 GeoIP 모듈 참조자
    std::atomic<uint32_t> next_user_id_{1};

    // 현재 서버에 접속 중인 모든 웹소켓 세션의 주소들을 관리하는 셋
    // 멀티 스레드 변경시 주의 필요.
    std::unordered_set<uWS::WebSocket<false, true, PerSocketData>*> clients_;

    // 그리드 칸 소유권: 빈 문자열 = 미점유. GRID_COLS*GRID_ROWS 크기의 1차원 배열로 관리.
    // 싱글스레드 이벤트 루프 위에서만 접근되므로 락이 필요 없음.
    std::array<std::string, static_cast<size_t>(GRID_COLS) * GRID_ROWS> grid_owner_;

    // 마지막 앰비언트 동기화 이후 새로 칠해진 칸의 인덱스 모음 (전역, 유저별 아님).
    // try_paint_cell에서 채우고, flush_dirty_cells에서 비움.
    std::unordered_set<uint32_t> dirty_cells_;

    // 틱마다 증가, BULK_SYNC_INTERVAL_MS 주기가 되면 0으로 리셋하며 dirty_cells_ 일괄 플러시
    int bulk_sync_tick_counter_ = 0;
};

#endif // GAME_SERVER_H