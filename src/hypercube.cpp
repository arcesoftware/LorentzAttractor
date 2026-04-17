#include "hypercube.h"
#include <vector>
#include <cmath>
#include <array>

// 5D hypercube: 32 vertices
std::vector<float> generate_5d_hypercube_lines(float angle) {
    std::vector<std::array<float,5>> verts;
    for (int i=0;i<32;i++){
        float v[5];
        for (int d=0;d<5;d++) v[d] = (i & (1<<d)) ? 0.5f : -0.5f;
        // rotate between dimension 4 and 0 for animation
        float ca = std::cos(angle), sa = std::sin(angle);
        float x = v[0], e4 = v[4];
        float xr = x*ca - e4*sa; float e4r = x*sa + e4*ca;
        verts.push_back({xr, v[1], v[2], v[3], e4r});
    }
    std::vector<float> lines;
    float d1 = 2.0f, d2 = 2.0f;
    for (int i=0;i<32;i++){
        for (int b=0;b<5;b++){
            int j = i ^ (1<<b);
            if (j <= i) continue;
            auto A = verts[i]; auto B = verts[j];
            // project 5D -> 4D by w4, then 4D -> 3D by w3 (use last coordinate as extra)
            float sA1 = 1.0f / (d1 - A[4]);
            float sB1 = 1.0f / (d1 - B[4]);
            std::array<float,4> A4 = {A[0]*sA1, A[1]*sA1, A[2]*sA1, A[3]*sA1};
            std::array<float,4> B4 = {B[0]*sB1, B[1]*sB1, B[2]*sB1, B[3]*sB1};
            float sA2 = 1.0f / (d2 - A4[3]);
            float sB2 = 1.0f / (d2 - B4[3]);
            float ax = A4[0]*sA2, ay = A4[1]*sA2, az = A4[2]*sA2;
            float bx = B4[0]*sB2, by = B4[1]*sB2, bz = B4[2]*sB2;
            lines.push_back(ax); lines.push_back(ay); lines.push_back(az);
            lines.push_back(bx); lines.push_back(by); lines.push_back(bz);
        }
    }
    return lines;
}
