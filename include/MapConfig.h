#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

// 등장방형 투영 기준의 전체 월드 좌표계. 클라이언트 화면(뷰포트)보다 훨씬 크며,
// 클라이언트는 이 중 내 캐릭터를 중심으로 한 일부분(카메라 뷰포트)만 잘라서 렌더링한다.
// 뷰포트(1600x800) 대비 3배 — 2배일 때는 AOI가 월드의 60~70%를 덮어 필터링 효과가 제한적이었음
inline constexpr int MAP_WIDTH = 4800;  // 경도 -180~180 -> 0~4800
inline constexpr int MAP_HEIGHT = 2400; // 위도 90~-90 -> 0~2400
inline constexpr int CELL_SIZE = 32;    // 칸 하나의 픽셀 크기. client/index.html의 drawPlayerCube size와 동일 —
                                         // 캐릭터가 위치한 칸 하나만 칠해지도록(GameServer::try_paint_cell) 일치시킴
inline constexpr int GRID_COLS = MAP_WIDTH / CELL_SIZE;   // 100
inline constexpr int GRID_ROWS = MAP_HEIGHT / CELL_SIZE;  // 50

// 클라이언트 카메라 뷰포트 크기 (client/index.html의 VIEWPORT_WIDTH/HEIGHT와 반드시 일치)
inline constexpr int VIEWPORT_WIDTH = 1600;
inline constexpr int VIEWPORT_HEIGHT = 800;

// AOI(관심 영역) 필터링: 뷰포트 밖 몇 칸까지를 "즉시 동기화 대상"으로 볼지
inline constexpr int AOI_MARGIN_CELLS = 5;

// 뷰포트 밖에서 쌓인 변경사항을 몇 ms마다 일괄(벌크) 동기화할지
inline constexpr int BULK_SYNC_INTERVAL_MS = 1000;

#endif // MAP_CONFIG_H
