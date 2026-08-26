# geoIP-cube-painter

GeoIP 기반 실시간 멀티플레이 웹 게임 서버. 접속 IP로 유저의 국가를 판별하고, 서버 권한형(Server-Authoritative) 이동 모델로 좌표를 계산해 실시간으로 브로드캐스트.

## 현재 상태

- WebSocket 기반 서버-클라이언트 연결, 국가 판별, 서버 권한형 이동 동기화까지 동작 확인됨
- 영역 점유("cube painting") 게임 로직은 진행 중

## 스택

- 서버: C++20, [uWebSockets](https://github.com/uNetworking/uWebSockets) / uSockets, [libmaxminddb](https://github.com/maxmind/libmaxminddb)(GeoIP 조회), vcpkg
- 클라이언트: `client/index.html` — WASD 입력, Canvas 렌더링, 바이너리 WebSocket 프로토콜

## 아키텍처

- 싱글스레드 이벤트 루프(uWebSockets 내장, epoll 기반) 위에서 네트워크 I/O와 게임 틱을 함께 처리
- 스레드를 분리하지 않아 락 경합을 원천 차단하는 구조를 의도적으로 선택 (멀티스레드 확장은 향후 과제로 코드 주석에 남겨둠)
- 자체 설계 바이너리 패킷 프로토콜(`include/Packets.h`): WELCOME / KEY_INPUT / KEY_ACK / MOVE_BROADCAST / USER_LEAVE

## 빌드 방법

vcpkg로 `uwebsockets`, `usockets`, `maxminddb`, `zlib` 설치 후:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake
make
```

## GeoIP 데이터베이스

`db/GeoLite2-Country.mmdb`는 라이선스 문제로 리포에 포함하지 않음. [MaxMind GeoLite2](https://dev.maxmind.com/geoip/geolite2-free-geolocation-data)에서 직접 다운로드해 `db/` 아래에 위치.

## 실행

```bash
./build/Server
```

기본 포트 7777. `client/index.html`을 브라우저로 열어 접속.
