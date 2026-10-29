#include <cstdio>
#include <string>
#include <exception>
#include <iostream>

import cx.core;
import cx.parse;
import cx.render;

int main(int argc, char** argv) {
    std::string expr = (argc > 1) ? argv[1] : "(z*z - 1)/(z*z + 4)";
    std::printf("Rendering: %s\n", expr.c_str());

    try {
        auto ast = cx::parse(expr);

        cx::RenderConfig cfg;
        cfg.width      = 1024; 
        cfg.height     = 1024; 
        cfg.max_depth  = 9;    
        cfg.half_width = 3.0;  
        cfg.show_grid  = true; // Naya feature ON

        cx::render_adaptive(*ast, cfg, "mobius.ppm");
        std::printf("Wrote mobius.ppm\n");
    }
    catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    return 0;
}