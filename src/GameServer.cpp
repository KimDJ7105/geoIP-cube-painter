#include "GameServer.h"
#include "GeoIPManager.h"
#include "Packets.h"
#include <iostream>
#include <string_view>

GameServer::GameServer(GeoIPManager& geoip_manager) : geoip_manager_(geoip_manager) {
    setup_behavior();
}

GameServer::~GameServer() {}

void GameServer::setup_behavior() {
    behavior_.compression = uWS::DISABLED;
    behavior_.maxPayloadLength = 16 * 1024;
    behavior_.idleTimeout = 16;
    behavior_.maxBackpressure = 1 * 1024 * 1024;
    behavior_.closeOnBackpressureLimit = false;

    /* 1. 커넥션 오픈 핸들러 (GeoIP 결합) */
    behavior_.open = [this](auto *ws) {
        // 클라이언트가 접근해서 웹 소켓 연결이 생성될때 실행된다. 
        std::string_view remote_address = ws->getRemoteAddressAsText();
        
        // 브로드 캐스팅 구독
        ws->subscribe("broadcast");

        std::string country = "LOCAL";
        // IPv4 로컬 호스트 및 IPv6 룹백 주소가 아닐 때만 국가 조회 수행
        if (remote_address != "127.0.0.1" && remote_address != "::1" && 
            remote_address.find("ffff:ac1b") == std::string_view::npos) { // 사설 매핑 대역 제외 예외처리
            country = geoip_manager_.get_country_code(remote_address);
        }

        //id 가져오고 1 증가
        uint32_t user_id = next_user_id_.fetch_add(1);

        // 유저 정보 구조체에 정보 저장
        PerSocketData *user_data = ws->getUserData();
        user_data->country = country;
        user_data->id = user_id;
        user_data->country = country;
        user_data->x = 0.0;
        user_data->y = 0.0;
        user_data->is_moving_up = false;
        user_data->is_moving_down = false;
        user_data->is_moving_left = false;
        user_data->is_moving_right = false;

        // 서버 전역 유저 목록에 소켓 등록
        clients_.insert(ws);

        // 접속시 본인의 id를 알려주는 패킷
        PacketWelcome welcome_packet;
        welcome_packet.type = PacketType::WELCOME;
        welcome_packet.userId = user_id; // 4바이트 매핑

        ws->send(std::string_view(reinterpret_cast<const char*>(&welcome_packet), sizeof(welcome_packet)), uWS::OpCode::BINARY);

        std::cout << "[모듈화 서버] 클라이언트 접속 | IP: " << remote_address 
                  << " | 국가: " << country << std::endl;
    };
    
    /* 2. 메시지 수신 핸들러 (고성능 바이너리 구조체 패킷 파싱) */
    behavior_.message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
        // 바이너리 패킷이 아니거나 최소 헤더 크기(1바이트)보다 작으면 차단
        if (opCode != uWS::OpCode::BINARY || message.length() < sizeof(PacketType)) {
            return;
        }

        PerSocketData *user_data = ws->getUserData();
        
        // 데이터의 첫 1바이트를 읽어 패킷 타입 확인
        PacketType type = *reinterpret_cast<const PacketType*>(message.data());

        if (type == PacketType::KEY_INPUT) {
            // 패킷 전체 크기 검증 (패킷 크기가 다르면 잘못된 패킷)
            if (message.length() < sizeof(PacketKeyInput)) return;

            // IOCP 스타일: 메모리 버퍼를 패킷 구조체 포인터로 고속 캐스팅 ($O(1)$)
            const PacketKeyInput* packet = reinterpret_cast<const PacketKeyInput*>(message.data());

            char key = static_cast<char>(packet->key_code);
            bool target_state = (packet->is_down == 1);

            switch (key) {
                case 'W': case 'w': user_data->is_moving_up = target_state; break;
                case 'S': case 's': user_data->is_moving_down = target_state; break;
                case 'A': case 'a': user_data->is_moving_left = target_state; break;
                case 'D': case 'd': user_data->is_moving_right = target_state; break;
                default: return;
            }

            std::cout << "[바이너리 입력 수신] 유저 ID: " << user_data->id 
                      << " | 키: " << key 
                      << " | 상태: " << (target_state ? "ON" : "OFF") << std::endl;

            // 응답 패킷 조립 및 바이너리 송신
            PacketKeyAck ack_packet;
            ack_packet.type = PacketType::KEY_ACK;
            ack_packet.key_code = packet->key_code;
            ack_packet.is_down = packet->is_down;

            // 구조체 메모리 영역을 string_view로 감싸서 바이너리로 통째로 전송
            ws->send(std::string_view(reinterpret_cast<const char*>(&ack_packet), sizeof(ack_packet)), opCode);
        }
    };
    
    /* 3. 커넥션 클로즈 핸들러 */
    behavior_.close = [this](auto *ws, int /*code*/, std::string_view /*message*/) {
        PerSocketData *user_data = ws->getUserData();
        uint32_t leave_user_id = user_data->id;

        // 풀에서 먼저 지우기 전에, 살아있는 다른 유저들에게 먼저 패킷을 전송합니다.
        char leave_buffer[5];
        leave_buffer[0] = static_cast<char>(PacketType::USER_LEAVE); // 5번 헤더
        std::memcpy(&leave_buffer[1], &leave_user_id, sizeof(leave_user_id)); // Little Endian으로 4바이트 카피

        std::string_view leave_payload(leave_buffer, 5);

        // 클라이언트 풀을 순회하며 다른 유저들에게 패킷 전송
        for (auto* client_ws : clients_) {
        if (client_ws != ws) { // 퇴장하는 자기 자신은 스킵
            client_ws->send(leave_payload, uWS::OpCode::BINARY);
        }
    }

        // 전송이 완벽히 끝난 후 소켓 포인터를 제거
        clients_.erase(ws);

        std::cout << "[모듈화 서버] 클라이언트 해제 완료 | ID: " << leave_user_id 
                  << " | 잔여 유저들에게 퇴장 패킷 100% 직통 전송 완료" << std::endl;
    };
}

void GameServer::setup_game_loop() {
    // 현재 스레드에서 돌아가고 있는 uWebSockets의 이벤트 루프 포인터를 획득
    struct us_loop_t *loop = (struct us_loop_t *) uWS::Loop::get();

    // 루프 내부에 고성능 네이티브 타이머를 생성
    struct us_timer_t *delay_timer = us_create_timer(loop, 0, 0);

    // 타이머 확장 데이터 영역(Ext)에 이 객체의 주소(this)를 박아넣어 람다 내부로 전달합니다.
    // uWS의 내부 구조상 유연한 데이터 전달을 위한 정석적인 방식입니다.
    *(GameServer**)us_timer_ext(delay_timer) = this;

    // 서버 틱 주기 설정
    int tick_ms = SERVER_TICK_RATE; 

    // 타이머가 만료될 때마다 실행될 콜백 함수 등록
    us_timer_set(delay_timer, [](struct us_timer_t *t) {
        // 전달받은 Ext 포인터로부터 GameServer 주소를 복원합니다.
        GameServer* self = *(GameServer**)us_timer_ext(t);
        
        // 초당 유저가 이동할 이동 속도 정의 (1초에 5.0 유닛 이동한다고 가정)
        
        double speed = PLAYER_MOVE_SPEED;
        double delta_time = SERVER_TICK_RATE / 1000.0; // 33ms 틱당 이동 거리 = 속도 * (33 / 1000.0)
        double move_distance = speed * delta_time;

        // 멀티 스레드 변경시 주의 필요
        for (auto* ws : self->clients_) {
            PerSocketData* user_data = ws->getUserData();
            bool is_changed = false;

            if (user_data->is_moving_up)    { user_data->y -= move_distance; is_changed = true; }
            if (user_data->is_moving_down)  { user_data->y += move_distance; is_changed = true; }
            if (user_data->is_moving_left)  { user_data->x -= move_distance; is_changed = true; }
            if (user_data->is_moving_right) { user_data->x += move_distance; is_changed = true; }

            // 좌표 변화가 일어난 유저가 있다면 실시간 연산 로그 출력
            if (is_changed) {
                std::cout << "[틱 업데이트] 유저 ID: " << user_data->id 
                          << " | 현재 좌표: (" << user_data->x << ", " << user_data->y << ")" << std::endl;
                
                PacketMoveBroadcast packet;
                packet.type = 3; // PacketType::MOVE_BROADCAST
                packet.userId = user_data->id;
                packet.x = user_data->x;
                packet.y = user_data->y;

                std::string_view broadcast_payload(reinterpret_cast<const char*>(&packet), sizeof(packet));

                // 본인에게 이동 사실 통보
                ws->send(broadcast_payload, uWS::OpCode::BINARY);

                // 현재 접속 중인 다른 클라이언트에게 브로드캐스팅
                // uWS 내부적으로 최적화된 zero-copy 브로드캐스팅
                ws->publish("broadcast", broadcast_payload, uWS::OpCode::BINARY);
            }
        }

    }, tick_ms, tick_ms); // 처음 대기 시간, 이후 반복 주기
}

bool GameServer::start(int port) {
    bool is_successful = false;

    uWS::App()
        .ws<PerSocketData>("/*", std::move(behavior_))
        .listen("0.0.0.0", port, [this, &is_successful, port](auto *listen_socket) {
            if (listen_socket) {
                std::cout << "GeoIP 게임 서버 엔진 정상 가동 (포트: " << port << ")" << std::endl;
                setup_game_loop(); // 루프 시작
                is_successful = true;
            } else {
                std::cerr << "포트 " << port << " 바인딩 실패" << std::endl;
            }
        })
        .run();

    return is_successful;
}