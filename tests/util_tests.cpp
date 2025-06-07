#include <cassert>
#include <cmath>
#include <iostream>

int lerp(int start, int end, float t) {
    return start + static_cast<int>(t * (end - start));
}

float easeOutQuad(float t) {
    return 1 - (1 - t) * (1 - t);
}

bool pointInRect(int px, int py, int rx, int ry, int rw, int rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

int main() {
    // lerp tests
    assert(lerp(0, 10, 0.0f) == 0);
    assert(lerp(0, 10, 1.0f) == 10);
    assert(lerp(10, 20, 0.5f) == 15);

    // easeOutQuad tests
    const float epsilon = 1e-5f;
    assert(std::abs(easeOutQuad(0.0f) - 0.0f) < epsilon);
    assert(std::abs(easeOutQuad(1.0f) - 1.0f) < epsilon);
    assert(std::abs(easeOutQuad(0.5f) - 0.75f) < epsilon);

    // pointInRect tests
    assert(pointInRect(5, 5, 0, 0, 10, 10));
    assert(!pointInRect(-1, -1, 0, 0, 10, 10));
    assert(pointInRect(10, 10, 0, 0, 10, 10));

    std::cout << "All util tests passed!" << std::endl;
    return 0;
}
