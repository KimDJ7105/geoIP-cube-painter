# geoIP-cube-painter

GeoIP 기반 실시간 멀티플레이 웹 게임 서버. 접속 IP로 유저의 국가를 판별해 그 국가의 대표 좌표에서 시작시키고, 서버 권한형(Server-Authoritative) 이동 모델로 좌표를 계산해 실시간 브로드캐스트. 이동한 자리를 자국 색으로 칠하며 영역을 넓혀가는 캐주얼 영역 점유 게임.

## 현재 상태

- WebSocket 기반 서버-클라이언트 연결, GeoIP 국가 판별, 서버 권한형 이동 동기화
- 국가별 대표 좌표(centroid)에서 스폰 — 로컬 접속(LOCAL/UNKNOWN)은 테스트 편의를 위해 매번 무작위 국가로 대체
- 영역 점유(트레일 페인팅) — 캐릭터가 지나간 자리를 자국 색으로 칠함. 화면에 보이는 캐릭터 크기와 실제로 칠하는 범위(발자국)를 의도적으로 분리해 트레일이 과하게 두꺼워지지 않도록 조정
- 카메라 뷰포트 — 전체 월드(3200x1600)가 화면(1600x800)보다 커서, 내 캐릭터를 중심으로 화면이 스크롤됨
- 전체 지도 오버레이 (M 키) — 지금까지 칠해진 전체 맵과 현재 보고 있는 뷰포트 위치를 한눈에 확인
- 국가별 점유율(%) 실시간 표시
- 신규 접속자에게는 기존 플레이어 위치/국가, 칠해진 그리드 전체를 접속 즉시 스냅샷으로 전송 (재접속/새 탭에서도 기존 상태 바로 확인 가능)

## 스택

- 서버: C++20, [uWebSockets](https://github.com/uNetworking/uWebSockets) / uSockets, [libmaxminddb](https://github.com/maxmind/libmaxminddb)(GeoIP 조회), vcpkg
- 클라이언트: `client/index.html` — WASD 입력, Canvas 렌더링, 바이너리 WebSocket 프로토콜

## 아키텍처

- 싱글스레드 이벤트 루프(uWebSockets 내장, epoll 기반) 위에서 네트워크 I/O와 게임 틱을 함께 처리
- 스레드를 분리하지 않아 락 경합을 원천 차단하는 구조를 의도적으로 선택 (멀티스레드 확장은 향후 과제로 코드 주석에 남겨둠)
- 자체 설계 바이너리 패킷 프로토콜(`include/Packets.h`): WELCOME / KEY_INPUT / KEY_ACK / MOVE_BROADCAST / USER_LEAVE / PAINT_CELL / PLAYER_INFO
- 색상은 네트워크로 전송하지 않음 — 서버는 2자리 국가 코드만 보내고, 클라이언트가 코드를 해시해 동일한 색을 결정적으로 계산(`countryColor()`)
- 그리드 소유권은 `std::array<std::string, GRID_COLS*GRID_ROWS>`로 서버 메모리에 보관 (싱글스레드라 락 불필요)
- 국가별 대표 좌표는 `include/CountryCentroids.h` / `src/CountryCentroids.cpp`에서 관리, 등장방형(Equirectangular) 투영으로 맵 픽셀 좌표 변환

## 알려진 개선 지점

- 신규 접속 시 그리드 스냅샷을 칸 하나당 패킷 하나씩(최대 5000개) 개별 전송 중 — 벌크 패킷으로 압축 전송하는 최적화 여지 있음
- Redis 기반 분산 상태 공유는 원래 기획(`docs`/설계 메모 참고)에 있었으나 일정상 스코프 아웃

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

기본 포트 7777. `client/index.html`을 정적 서버로 띄워 브라우저로 접속 (예: `python3 -m http.server 8080`). WASD로 이동, M 키로 전체 지도 보기.
