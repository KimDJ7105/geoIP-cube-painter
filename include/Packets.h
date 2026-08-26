#ifndef PACKETS_H
#define PACKETS_H

#include <cstdint>

#pragma pack(push, 1) // 1바이트 단위 패킷 정렬

// 패킷의 종류를 식별할 열거형 헤더 (1바이트)
enum class PacketType : uint8_t {
    KEY_INPUT = 1,  // 클라이언트 -> 서버: 키 상태 변경 패킷
    KEY_ACK   = 2,   // 서버 -> 클라이언트: 확인 응답 패킷
    MOVE_BROADCAST = 3, // 서버 -> 클라이언트 : 이동 브로드캐스트 패킷
    WELCOME = 4,   // 서버 -> 클라이언트: 최초 접속 유저 고유 ID 할당 패킷
    USER_LEAVE = 5   // 서버 -> 클라이언트: 유저 퇴장 알림 패킷
};

// 클라이언트가 보내올 키 입력 패킷 구조체
struct PacketKeyInput {
    PacketType type;      // PacketType::KEY_INPUT (1바이트)
    uint8_t    key_code;  // 'W'(87), 'A'(65), 'S'(83), 'D'(68) ASCII 코드 (1바이트)
    uint8_t    is_down;   // 1: 누름(On), 0: 뗌(Off) (1바이트)
};

// 서버가 클라이언트에게 돌려줄 ACK 패킷 구조체
struct PacketKeyAck {
    PacketType type;      // PacketType::KEY_ACK (1바이트)
    uint8_t    key_code;  // 어떤 키에 대한 응답인지 (1바이트)
    uint8_t    is_down;   // 변경된 상태 (1바이트)
};

struct PacketMoveBroadcast {
    uint8_t type;       // PacketType::MOVE_BROADCAST (예: 3)
    uint32_t userId;    // 움직인 유저의 고유 ID (4바이트)
    float x;            // x 좌표 (4바이트)
    float y;            // y 좌표 (4바이트)
};

// 최초 접속 시 고유 ID 통보용 패킷 구조체 
struct PacketWelcome {
    PacketType type;    // PacketType::WELCOME (1바이트)
    uint32_t userId;    // 유저의 id
};

// 유저 퇴장 브로드캐스트 패킷 (총 9바이트)
struct PacketUserLeave {
    PacketType type;    // PacketType::USER_LEAVE (1바이트)
    uint32_t userId;    // 연결이 끊어진 유저의 고유 ID (8바이트)
};

#pragma pack(pop) // 바이트 정렬 원상 복구

#endif // PACKETS_H