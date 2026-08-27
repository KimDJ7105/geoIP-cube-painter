#ifndef PER_SOCKET_DATA_H
#define PER_SOCKET_DATA_H

#include <string>
#include <cstdint>

// 클라이언트 유저 컨텍스트 구조체
struct PerSocketData {
    uint64_t id;         // 서버가 발급하는 고유 유저 ID
    std::string country; // 식별된 접속 국가 코드

    // 실시간 2D 게임 좌표
    double x = 0.0;
    double y = 0.0;

    // 현재 유저의 키 입력 누름 상태 (True: 누름, False: 뗌)
    bool is_moving_up    = false; // W 키
    bool is_moving_down  = false; // S 키
    bool is_moving_left  = false; // A 키
    bool is_moving_right = false; // D 키

    // 마지막 주기적 보정 이후 한 번이라도 움직였는지. 움직였다면 이미 실시간(AOI 즉시 전송)으로
    // 커버되므로 주기적 보정 대상에서 제외 — 가만히 있는 유저만 보정 대상이 되도록 좁히기 위함.
    bool moved_since_last_sync = false;
};

#endif // PER_SOCKET_DATA_H