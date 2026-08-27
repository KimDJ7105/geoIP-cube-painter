#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

// 등장방형 투영 기준의 전체 월드 좌표계. 클라이언트 화면(뷰포트)보다 훨씬 크며,
// 클라이언트는 이 중 내 캐릭터를 중심으로 한 일부분(카메라 뷰포트)만 잘라서 렌더링한다.
inline constexpr int MAP_WIDTH = 3200;  // 경도 -180~180 -> 0~3200
inline constexpr int MAP_HEIGHT = 1600; // 위도 90~-90 -> 0~1600
inline constexpr int CELL_SIZE = 32;    // 칸 하나의 픽셀 크기 (플레이어보다 뚜렷하게 크게)
inline constexpr int GRID_COLS = MAP_WIDTH / CELL_SIZE;   // 100
inline constexpr int GRID_ROWS = MAP_HEIGHT / CELL_SIZE;  // 50

#endif // MAP_CONFIG_H
