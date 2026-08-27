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
    USER_LEAVE = 5,   // 서버 -> 클라이언트: 유저 퇴장 알림 패킷
    PAINT_CELL = 6,   // 서버 -> 클라이언트: 칸 하나가 칠해짐 (AOI 안에 있는 유저에게 즉시 개별 전송)
    PLAYER_INFO = 7,  // 서버 -> 클라이언트: 유저의 국가 정보(접속 시 1회 + 스냅샷용, 이동마다 보내는 MOVE_BROADCAST와 분리해 대역폭 절약)
    PAINT_CELL_BULK = 8, // 서버 -> 클라이언트: 여러 칸을 한 번에 전송 (접속 시 전체 스냅샷 + AOI 밖 변경사항 주기적/요청 동기화)
    MAP_REQUEST = 9   // 클라이언트 -> 서버: 전체 지도(M 키) 열람 시, 밀린 변경사항을 즉시 벌크로 달라는 요청
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

// 칸 색칠 패킷 (신규 페인팅 브로드캐스트 겸, 접속 시 그리드 스냅샷 전송에도 재사용) (총 7바이트)
struct PacketPaintCell {
    PacketType type;      // PacketType::PAINT_CELL (1바이트)
    uint16_t cellX;        // 그리드 칸 x 인덱스 (2바이트)
    uint16_t cellY;        // 그리드 칸 y 인덱스 (2바이트)
    char countryCode[2];   // 칠한 유저의 국가 코드, 2자리로 정규화됨 (2바이트)
};

// 유저 국가 정보 패킷 (접속 시 1회 전송 + 신규 접속자용 스냅샷에 재사용) (총 7바이트)
struct PacketPlayerInfo {
    PacketType type;      // PacketType::PLAYER_INFO (1바이트)
    uint32_t userId;       // 대상 유저 ID (4바이트)
    char countryCode[2];   // 2자리로 정규화된 국가 코드 (2바이트)
};

// PAINT_CELL_BULK 레이아웃 (가변 길이, 구조체 대신 직접 바이트를 조립/파싱):
//   [0]     PacketType (1바이트, PAINT_CELL_BULK)
//   [1..2]  entryCount (uint16, 리틀 엔디안)
//   [3..]   entryCount개의 항목, 항목당 6바이트: cellX(uint16) cellY(uint16) countryCode(2)
// 총 길이 = 3 + entryCount * 6

// MAP_REQUEST: 클라이언트 -> 서버, 헤더 1바이트(PacketType)만 있는 요청 패킷. 별도 구조체 불필요.

#pragma pack(pop) // 바이트 정렬 원상 복구

#endif // PACKETS_H