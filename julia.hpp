#ifndef JULIA_HPP
#define JULIA_HPP

#include "GUI_Paint.h"
#include "DEV_Config.h"
#include <vector>
#include <tuple>

class JuliaSet {
public:
    JuliaSet(double cx = -0.7, double cy = 0.27015);
    
    void SetRender(UBYTE* image);
    void Render(UWORD xResolution, UWORD yResolution);
    bool IsJuliaPoint(double fX, double fY, int iterations);
    unsigned long long GetUniformnessOfArea(double fW, double fH, int xOffset, int yOffset, int wDiv, int hDiv);
    bool IsAreaUniform(int xOffset, int yOffset, double fW, double fH, int wDiv, int hDiv, double wStart, double hStart);
    double GetImprovedUniformnessOfArea(double fW, double fH, int xOffset, int yOffset, int wDiv, int hDiv);
    void ZoomOnInterestingArea();

private:
    double c_x, c_y; 
    double w, h, x, y;
    UBYTE* rendered;
    UWORD renderedResX, renderedResY;
};

#endif
