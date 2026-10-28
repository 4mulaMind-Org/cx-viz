module;
#include <vector>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <string>
#include <cmath>
#include <numbers>
#include <queue>
#include <iostream>
#include <mdspan>

export module cx.render;
import cx.core;
import cx.parse;
import cx.spatial;

export namespace cx {

struct RenderConfig {
    int width  = 1024;
    int height = 1024;
    double center_x = 0.0;
    double center_y = 0.0;
    double half_width = 3.0;
    int max_depth = 9;
};

inline void render_adaptive(
    const Node& f, const RenderConfig& cfg, const std::string& path)
{
    std::vector<std::uint32_t> raw_pixels(static_cast<std::size_t>(cfg.width) * cfg.height);
    std::mdspan<std::uint32_t, std::dextents<int, 2>> img(raw_pixels.data(), cfg.height, cfg.width);

    double px_to_world = (2.0 * cfg.half_width) / cfg.width;
    auto to_world = [&](int px, int py) {
        double x = cfg.center_x + (px - cfg.width  * 0.5) * px_to_world;
        double y = cfg.center_y - (py - cfg.height * 0.5) * px_to_world;
        return Complex<>{x, y};
    };

    std::priority_queue<Tile> pq;
    pq.push({0, 0, cfg.width, 0.0});
    int tiles_processed = 0;

    while (!pq.empty()) {
        auto t = pq.top();
        pq.pop();
        tiles_processed++;

        int x = t.x, y = t.y, size = t.size;
        int depth = (int)std::log2(cfg.width / size);

        if (depth >= cfg.max_depth || size <= 1) {
            for (int dy = 0; dy < size; ++dy) {
                for (int dx = 0; dx < size; ++dx) {
                    auto c = get_rgb(eval(f, to_world(x + dx, y + dy)));
                    img[y + dy, x + dx] = c;
                }
            }
            continue;
        }

        auto c_tl = get_rgb(eval(f, to_world(x, y)));
        auto c_tr = get_rgb(eval(f, to_world(x + size - 1, y)));
        auto c_bl = get_rgb(eval(f, to_world(x, y + size - 1)));
        auto c_br = get_rgb(eval(f, to_world(x + size - 1, y + size - 1)));
        auto c_c  = get_rgb(eval(f, to_world(x + size / 2, y + size / 2)));

        double err = color_diff(c_tl, c_tr) + color_diff(c_bl, c_br) + 
                     color_diff(c_tl, c_bl) + color_diff(c_tr, c_br) +
                     color_diff(c_tl, c_c)  + color_diff(c_tr, c_c) +
                     color_diff(c_bl, c_c)  + color_diff(c_br, c_c);

        if (err > 50.0) {
            // BULLETPROOF SPLIT: Koi pixel miss nahi hoga
            int half1 = size / 2;
            int half2 = size - half1;
            pq.push({x, y, half1, err});
            pq.push({x + half1, y, half2, err});
            pq.push({x, y + half1, half1, err});
            pq.push({x + half1, y + half1, half2, err});
        } else {
            for (int dy = 0; dy < size; ++dy) {
                for (int dx = 0; dx < size; ++dx) {
                    auto c = get_rgb(eval(f, to_world(x + dx, y + dy)));
                    img[y + dy, x + dx] = c;
                }
            }
        }
    }

    static bool printed = false;
    if (!printed) {
        std::cout << "Tiles processed: " << tiles_processed << "\n";
        printed = true;
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("cannot open output: " + path);
    out << "P6\n" << cfg.width << " " << cfg.height << "\n255\n";
    for (auto p : raw_pixels) {
        out.put(static_cast<char>((p >> 16) & 0xFF));
        out.put(static_cast<char>((p >>  8) & 0xFF));
        out.put(static_cast<char>( p        & 0xFF));
    }
}

} // namespace cx
