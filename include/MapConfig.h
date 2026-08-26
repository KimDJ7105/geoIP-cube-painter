#ifndef MAP_CONFIG_H
#define MAP_CONFIG_H

// 클라이언트 캔버스와 1:1로 대응하는 월드 좌표계 (등장방형 투영)
inline constexpr int MAP_WIDTH = 800;   // 경도 -180~180 -> 0~800
inline constexpr int MAP_HEIGHT = 400;  // 위도 90~-90 -> 0~400
inline constexpr int CELL_SIZE = 20;    // 칸 하나의 픽셀 크기
inline constexpr int GRID_COLS = MAP_WIDTH / CELL_SIZE;   // 40
inline constexpr int GRID_ROWS = MAP_HEIGHT / CELL_SIZE;  // 20

#endif // MAP_CONFIG_H
