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
    bool show_grid = true;
};

inline void render_adaptive(
    const Node& f, const RenderConfig& cfg, const std::string& path)
{
    std::vector<std::uint32_t> raw_pixels(static_cast<std::size_t>(cfg.width) * cfg.height);
    std::mdspan<std::uint32_t, std::dextents<int, 2>> img(raw_pixels.data(), cfg.height, cfg.width);

    double px_to_world = (2.0 * cfg.half_width) / cfg.width;
    
    // Möbius Transformation: w = (z-1)/(z+1)
    Complex<> ma{1.0, 0.0}, mb{-1.0, 0.0}, mc{1.0, 0.0}, md{1.0, 0.0};

    auto to_world = [&](int px, int py) {
        double x = cfg.center_x + (px - cfg.width  * 0.5) * px_to_world;
        double y = cfg.center_y - (py - cfg.height * 0.5) * px_to_world;
        
        Complex<> z{x, y};
        
        // Apply Möbius
        return apply_mobius(z, ma, mb, mc, md);
    };

    std::priority_queue<Tile> pq;
    pq.push({0, 0, cfg.width, 0.0});
    int tiles_processed = 0;
    std::vector<Complex<>> detected_singularities;

    while (!pq.empty()) {
        auto t = pq.top();
        pq.pop();
        tiles_processed++;

        int x = t.x, y = t.y, size = t.size;
        int depth = (int)std::log2(cfg.width / size);

        // 1. Pole/Zero Detection Check 
        if (size <= 16 && depth >= 4) {
            Complex<> center = to_world(x + size / 2, y + size / 2);
            double radius = (size * 0.75) * px_to_world; 

            double delta = get_total_delta(f, center, radius);
            
            if (std::abs(delta) > 3.14) {
                bool already_detected = false;
                for (const auto& p : detected_singularities) {
                    if (std::abs(p.re - center.re) < 0.2 && std::abs(p.im - center.im) < 0.2) {
                        already_detected = true;
                        break;
                    }
                }

                if (!already_detected) {
                    detected_singularities.push_back(center);
                    for(int dy=0; dy<size; dy++) {
                        for(int dx=0; dx<size; dx++) {
                            img[y+dy, x+dx] = 0xFF0000; 
                        }
                    }
                }
                continue;
            }
        }

        // 2. Agar max depth aa gayi...
        if (depth >= cfg.max_depth || size <= 1) {
            for (int dy = 0; dy < size; ++dy) {
                for (int dx = 0; dx < size; ++dx) {
                    Complex<> z = to_world(x + dx, y + dy);
                    
                    if (cfg.show_grid) {
                        double gx = std::abs(z.re - std::round(z.re));
                        double gy = std::abs(z.im - std::round(z.im));
                        // px_to_world * 1.5 ensures 1-2 pixel thin line
                        if (gx < px_to_world * 1.5 || gy < px_to_world * 1.5) {
                            img[y+dy, x+dx] = 0x444444; // Thoda lighter grey
                            continue;
                        }
                    }
                    
                    auto c = get_rgb(eval(f, z));
                    img[y + dy, x + dx] = c;
                }
            }
            continue;
        }

        // 3. Quadtree Error Calculation
        auto c_tl = get_rgb(eval(f, to_world(x, y)));
        auto c_tr = get_rgb(eval(f, to_world(x + size - 1, y)));
        auto c_bl = get_rgb(eval(f, to_world(x, y + size - 1)));
        auto c_br = get_rgb(eval(f, to_world(x + size - 1, y + size - 1)));
        auto c_c  = get_rgb(eval(f, to_world(x + size / 2, y + size / 2)));

        double err = color_diff(c_tl, c_tr) + color_diff(c_bl, c_br) + 
                     color_diff(c_tl, c_bl) + color_diff(c_tr, c_br) +
                     color_diff(c_tl, c_c)  + color_diff(c_tr, c_c) +
                     color_diff(c_bl, c_c)  + color_diff(c_br, c_c);

        // 4. Split ya Fill (Threshold 50 se 10 kar diya)
        if (err > 10.0 || size > 16) {
            int half1 = size / 2;
            int half2 = size - half1;
            pq.push({x, y, half1, err});
            pq.push({x + half1, y, half2, err});
            pq.push({x, y + half1, half1, err});
            pq.push({x + half1, y + half1, half2, err});
        } else {
            for (int dy = 0; dy < size; ++dy) {
                for (int dx = 0; dx < size; ++dx) {
                    Complex<> z = to_world(x + dx, y + dy);
                    
                    if (cfg.show_grid) {
                        double gx = std::abs(z.re - std::round(z.re));
                        double gy = std::abs(z.im - std::round(z.im));
                        // px_to_world * 1.5 ensures 1-2 pixel thin line
                        if (gx < px_to_world * 1.5 || gy < px_to_world * 1.5) {
                            img[y+dy, x+dx] = 0x444444; // Thoda lighter grey
                            continue;
                        }
                    }
                    
                    auto c = get_rgb(eval(f, z));
                    img[y + dy, x + dx] = c;
                }
            }
        }
    }

    std::cout << "Unique Singularities: " << detected_singularities.size() << "\n";
    std::cout << "Tiles processed: " << tiles_processed << "\n";

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