# include <Siv3D.hpp>
# include "DebugTools.hpp"
# include "Layout.hpp"

static Rect from_points(int x1, int y1, int x2, int y2){
    assert(x1 < x2);
    assert(y1 < y2);
    return Rect{x1, y1, x2 - x1, y2 - y1};
}

static Rect operator+(const Rect& a, const Point& delta){
    return Rect{a.pos + delta, a.size};
}

int test_main(){
    INFO("Test Start");
    Rect area{0, 0, 300, 400};
    assert(from_points(0, 0, 300, 200)     == cliped_Y(area, 0, 0.5));
    assert(from_points(0, 200, 300, 400)   == cliped_Y(area, 0.5, 1));
    assert(from_points(0, 0, 150, 400)     == cliped_X(area, 0, 0.5));
    assert(from_points(150, 0, 300, 400)   == cliped_X(area, 0.5, 1.0));

    // 位置不変性
    Point delta{50, 50};
    assert(from_points(0, 0, 300, 200)      + delta == cliped_Y(area + delta, 0, 0.5));
    assert(from_points(0, 200, 300, 400)    + delta == cliped_Y(area + delta, 0.5, 1));
    assert(from_points(0, 0, 150, 400)      + delta == cliped_X(area + delta, 0, 0.5));
    assert(from_points(150, 0, 300, 400)    + delta == cliped_X(area + delta, 0.5, 1.0));

    INFO("Test End");
    return 0;
}