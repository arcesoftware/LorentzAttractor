#include "tesseract.h"
#include <vector>
#include <cmath>
#include <array>

// Generate 16 vertices of 4D hypercube and edges between vertices that differ by 1 bit
std::vector<float> generate_tesseract_lines(float angle) {
    std::vector<std::array<float,4>> verts;
    for (int i=0;i<16;i++){
        float x = (i & 1) ? 0.5f : -0.5f;
        float y = (i & 2) ? 0.5f : -0.5f;
        float z = (i & 4) ? 0.5f : -0.5f;
        float w = (i & 8) ? 0.5f : -0.5f;
        // rotate in w-x plane for animation
        float ca = std::cos(angle), sa = std::sin(angle);
        float xr = x * ca - w * sa;
        float wr = x * sa + w * ca;
        verts.push_back({xr,y,z,wr});
    }
    // edges: pairs of indices differing by one bit
    std::vector<float> lines;
    float projDist = 2.0f;
    for (int i=0;i<16;i++){
        for (int b=0;b<4;b++){
            int j = i ^ (1<<b);
            if (j <= i) continue;
            auto A = verts[i]; auto B = verts[j];
            // project 4D (x,y,z,w) -> 3D by perspective on w
            float sA = 1.0f / (projDist - A[3]);
            float sB = 1.0f / (projDist - B[3]);
            float ax = A[0]*sA, ay = A[1]*sA, az = A[2]*sA;
            float bx = B[0]*sB, by = B[1]*sB, bz = B[2]*sB;
            lines.push_back(ax); lines.push_back(ay); lines.push_back(az);
            lines.push_back(bx); lines.push_back(by); lines.push_back(bz);
        }
    }
    return lines;
}
