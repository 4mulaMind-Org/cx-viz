#include <cstdio>
#include <string>
#include <exception>
#include <vector>
#include <iostream>
#include <cmath>

import cx.core;
import cx.parse;
import cx.render;

int main(int argc, char** argv) {
    std::string expr = (argc > 1) ? argv[1] : "(z*z - 1)/(z*z + 4)";
    std::printf("Rendering animation for: %s\n", expr.c_str());

    try {
        auto ast = cx::parse(expr);

        cx::RenderConfig cfg;
        cfg.width      = 1024; 
        cfg.height     = 1024; 
        cfg.max_depth  = 9;    

        double start_y = 0.0;
        double end_y   = 2.0; 
        
        int frames = 30; 

        for (int i = 0; i < frames; ++i) {
            double t = (double)i / (frames - 1);
            double ease_t = t * t * (3.0 - 2.0 * t); 
            
            cfg.center_y = start_y + (end_y - start_y) * ease_t;
            cfg.half_width = 3.0 * std::pow(0.05, ease_t); 

            char filename[64];
            std::snprintf(filename, sizeof(filename), "frames/frame_%03d.ppm", i);

            cx::render_adaptive(*ast, cfg, filename);
        }
        
        std::printf("Animation complete! %d frames generated in 'frames/' folder.\n", frames);
    }
    catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    return 0;
}
