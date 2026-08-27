#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

// 등장방형 투영 기준의 전체 월드 좌표계. 클라이언트 화면(뷰포트)보다 훨씬 크며,
// 클라이언트는 이 중 내 캐릭터를 중심으로 한 일부분(카메라 뷰포트)만 잘라서 렌더링한다.
inline constexpr int MAP_WIDTH = 3200;  // 경도 -180~180 -> 0~3200
inline constexpr int MAP_HEIGHT = 1600; // 위도 90~-90 -> 0~1600
inline constexpr int CELL_SIZE = 32;    // 칸 하나의 픽셀 크기 (플레이어보다 뚜렷하게 크게)
inline constexpr int GRID_COLS = MAP_WIDTH / CELL_SIZE;   // 100
inline constexpr int GRID_ROWS = MAP_HEIGHT / CELL_SIZE;  // 50

// 플레이어가 실제로 칠하는 정사각형(발자국) 크기.
// 의도적으로 캐릭터 시각적 렌더링 크기(client/index.html의 drawPlayerCube size=56)보다 작게 잡음 —
// 보이는 크기와 칠하는 범위를 분리해서, 이동 중 칠해지는 궤적이 너무 두꺼워지지 않도록 함
inline constexpr int PLAYER_FOOTPRINT = 24;

// 클라이언트 카메라 뷰포트 크기 (client/index.html의 VIEWPORT_WIDTH/HEIGHT와 반드시 일치)
inline constexpr int VIEWPORT_WIDTH = 1600;
inline constexpr int VIEWPORT_HEIGHT = 800;

// AOI(관심 영역) 필터링: 뷰포트 밖 몇 칸까지를 "즉시 동기화 대상"으로 볼지
inline constexpr int AOI_MARGIN_CELLS = 5;

// 뷰포트 밖에서 쌓인 변경사항을 몇 ms마다 일괄(벌크) 동기화할지
inline constexpr int BULK_SYNC_INTERVAL_MS = 1000;

#endif // MAP_CONFIG_H
