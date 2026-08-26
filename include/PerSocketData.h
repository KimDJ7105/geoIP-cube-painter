#ifndef PER_SOCKET_DATA_H
#define PER_SOCKET_DATA_H

#include <string>

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
};

#endif // PER_SOCKET_DATA_H