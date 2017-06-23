#pragma once
#include "game_math.h"
#include "geometry.h"

#pragma pack(1)
reflectable struct GridCell {
  i32 x;
  i32 y;
};

//
//      | 0,1 |
//   ---|-----|---
// -1,0 | 0,0 | 1,0
//   ---|-----|---
//      | 0,-1|
// (0,0 == origin)
//
reflectable struct Grid {
  vec2 origin;
  f32 cell_width;
};

inline bool operator==(const GridCell& lhs, const GridCell& rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const GridCell& lhs, const GridCell& rhs) {
  return !(lhs.x == rhs.x && lhs.y == rhs.y);
}

inline f32 cell_distance(GridCell lhs, GridCell rhs) {
  return magnitude(vec2{ (f32)lhs.x, (f32)lhs.y } -vec2{ (f32)rhs.x, (f32)rhs.y });
}

inline i32 cell_taxicab_distance(GridCell lhs, GridCell rhs) {
  return _abs(lhs.x - rhs.x) + _abs(lhs.y - rhs.y);
}

inline GridCell position_to_cell(Grid grid, vec2 p) {
  vec2 prel = p - grid.origin;
  i32 x = _round(prel.x / grid.cell_width);
  i32 y = _round(prel.y / grid.cell_width);
  return GridCell{ x, y };
}

inline vec2 cell_to_position(Grid grid, GridCell cell) {
  return vec2{
    (f32)cell.x * grid.cell_width + grid.origin.x,
    (f32)cell.y * grid.cell_width + grid.origin.y
  };
}

inline vec2 cell_top_right(Grid grid, GridCell cell) {
  return vec2{
    ((f32)cell.x + 0.5f) * grid.cell_width + grid.origin.x,
    ((f32)cell.y + 0.5f) * grid.cell_width + grid.origin.y
  };
}

inline vec2 cell_bottom_left(Grid grid, GridCell cell) {
  return vec2{
    ((f32)cell.x - 0.5f) * grid.cell_width + grid.origin.x,
    ((f32)cell.y - 0.5f) * grid.cell_width + grid.origin.y
  };
}

inline vec2 snap_to_grid(Grid grid, vec2 p) {
  return cell_to_position(grid, position_to_cell(grid, p));
}

inline vec2 snap_to_bottom_left(Grid grid, vec2 p) {
  return cell_bottom_left(grid, position_to_cell(grid, p));
}

inline vec2 snap_to_top_right(Grid grid, vec2 p) {
  return cell_top_right(grid, position_to_cell(grid, p));
}

inline f32 cell_left(Grid grid, GridCell cell) {
  return (f32)cell.x * grid.cell_width + grid.origin.x - grid.cell_width / 2.0f;
}

inline f32 cell_right(Grid grid, GridCell cell) {
  return (f32)cell.x * grid.cell_width + grid.origin.x + grid.cell_width / 2.0f;
}

inline f32 cell_top(Grid grid, GridCell cell) {
  return (f32)cell.y * grid.cell_width + grid.origin.y + grid.cell_width / 2.0f;
}

inline f32 cell_bottom(Grid grid, GridCell cell) {
  return (f32)cell.y * grid.cell_width + grid.origin.y - grid.cell_width / 2.0f;
}
