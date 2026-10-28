module;
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <cstdint>

export module cx.spatial;
import cx.core;
import cx.parse;

export namespace cx {

struct Tile {
    int x, y, size;
    double error;
    
    // Priority Queue ko batana hai ki sabse bada error pehle nikle
    bool operator<(const Tile& other) const {
        return error < other.error; 
    }
};

// Pixel ka color calculate karna (helper)
inline std::uint32_t get_rgb(Complex<> w) {
    double hue  = w.arg() * 180.0 / 3.14159265358979323846;
    double mag  = std::log1p(w.abs());
    double v    = 0.5 + 0.5 * std::sin(mag * 2.0 * 3.14159265358979323846);
    
    // HSV to RGB (short version)
    hue = std::fmod(hue, 360.0); if (hue < 0) hue += 360.0;
    double c = v * 0.9;
    double x = c * (1.0 - std::fabs(std::fmod(hue / 60.0, 2.0) - 1.0));
    double m = v - c;
    double r=0, g=0, b=0;
    if      (hue <  60) { r=c; g=x; }
    else if (hue < 120) { r=x; g=c; }
    else if (hue < 180) { g=c; b=x; }
    else if (hue < 240) { g=x; b=c; }
    else if (hue < 300) { r=x; b=c; }
    else                { r=c; b=x; }
    auto q = [&](double f) {
        return static_cast<std::uint32_t>(std::clamp((f + m) * 255.0, 0.0, 255.0));
    };
    return (q(r) << 16) | (q(g) << 8) | q(b);
}

inline double color_diff(std::uint32_t c1, std::uint32_t c2) {
    int r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    int r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    return std::abs(r1 - r2) + std::abs(g1 - g2) + std::abs(b1 - b2);
}

} // namespace cx
