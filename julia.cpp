#include "julia.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>

using namespace std;

JuliaSet::JuliaSet(double cx, double cy)
    : c_x(cx), c_y(cy), w(4), h(2), x(0), y(0), rendered(nullptr), renderedResX(0), renderedResY(0) {}

void JuliaSet::SetRender(UBYTE* image) {
    rendered = image;
}

void JuliaSet::Render(UWORD xResolution, UWORD yResolution) {
    int iter = (100 + max(0.0, -log10(w)) * 200); 
    vector<vector<bool>> columns;

    for (int i = yResolution - 1; i >= 0; --i) {
        vector<bool> rows;
        for (int j = 0; j < xResolution; ++j) {
            double p_x = this->x - this->w / 2.0 + (double)j / (double)xResolution * this->w;
            double p_y = this->y - this->h / 2.0 + (double)(i + 1) / (double)yResolution * this->h;
            rows.emplace_back(IsJuliaPoint(p_x, p_y, iter)); 
        }
        columns.emplace_back(rows);
    }

    renderedResX = xResolution;
    renderedResY = yResolution;

    for (unsigned int y = 0; y < columns.size(); ++y) {
        auto row = columns[y];
        for (unsigned int x = 0; x < row.size(); ++x) {
            auto bitSet = row[x];
            if (bitSet) {
                Paint_SetPixel(x, y, WHITE);
            } else {
                Paint_SetPixel(x, y, BLACK);
            }
        }
    }
}

bool JuliaSet::IsJuliaPoint(double z_x, double z_y, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        double z_x_old = z_x;
        z_x = z_x * z_x - z_y * z_y + c_x;
        z_y = 2.0 * z_x_old * z_y + c_y;
        if (z_x * z_x + z_y * z_y > 4) {
            return true;
        }
    }
    return false;
}

unsigned long long JuliaSet::GetUniformnessOfArea(double fW, double fH, int xOffset, int yOffset, int wDiv, int hDiv) {
    unsigned long long uniformness = 0;
    for (int wStart = 0; wStart < wDiv; ++wStart) {
        for (int hStart = 0; hStart < hDiv; ++hStart) {
            if (IsAreaUniform(xOffset, yOffset, fW, fH, wDiv, hDiv, wStart, hStart)) {
                ++uniformness;
            }
        }
    }

    return uniformness;
}

bool JuliaSet::IsAreaUniform(int xOffset, int yOffset, double fW, double fH, int wDiv, int hDiv, double wStart, double hStart) {
    int yInit = yOffset + static_cast<int>(fH / hDiv) * hStart;
    int xInit = xOffset + static_cast<int>(fW / wDiv) * wStart;
    auto firstPoint = Paint_GetPixel(xInit, yInit);

    for (unsigned int i = 0; i < static_cast<unsigned int>(fW / wDiv); ++i) {
        for (unsigned int j = 0; j < static_cast<unsigned int>(fH / hDiv); ++j) {
            int yTest = yOffset + static_cast<int>(fH / hDiv) * hStart + j;
            int xTest = xOffset + static_cast<int>(fW / wDiv) * wStart + i;
            auto testPoint = Paint_GetPixel(xTest, yTest);
            if (testPoint != firstPoint)
                return false;
        }
    }

    return true;
}

double JuliaSet::GetImprovedUniformnessOfArea(double fW, double fH, int xOffset, int yOffset, int wDiv, int hDiv) {
    unsigned long long numWhite = 0;
    unsigned long long numBlack = 0;
    double totalPixels = fW * fH;

    for (int wStart = 0; wStart < fW; ++wStart) {
        for (int hStart = 0; hStart < fH; ++hStart) {
            int xPointIndex = xOffset + wStart;
            int yPointIndex = yOffset + hStart;
            if (Paint_GetPixel(xPointIndex, yPointIndex) == WHITE)
                numWhite++;
            else
                numBlack++;
        }
    }

    return max((double)numWhite / totalPixels, (double)numBlack / totalPixels);
}

void JuliaSet::ZoomOnInterestingArea() {

    tuple<double, double, double> choice;
    vector<tuple<double, double, double>> choices;

    auto uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, 0, 0, 2, 2);
    choice = {this->x - this->w / 4, this->y + this->h / 4, uniformness};
    choices.emplace_back(choice);

    uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, this->renderedResX / 2, 0, 2, 2);
    choice = {this->x + this->w / 4, this->y + this->h / 4, uniformness};
    choices.emplace_back(choice);

    uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, 0, this->renderedResY / 2, 2, 2);
    choice = {this->x - this->w / 4, this->y - this->h / 4, uniformness};
    choices.emplace_back(choice);

    uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, this->renderedResX / 2, this->renderedResY / 2, 2, 2);
    choice = {this->x + this->w / 4, this->y - this->h / 4, uniformness};
    choices.emplace_back(choice);

    w = w / 2.0;
    h = h / 2.0;

    auto lessUniformChoices = choices;
    lessUniformChoices.erase(std::remove_if(
        lessUniformChoices.begin(),
        lessUniformChoices.end(),
        [](const tuple<double, double, double>& x) {
            return (std::get<2>(x) >= 0.95);
        }), lessUniformChoices.end());

    auto topTierChoices = choices;
    topTierChoices.erase(std::remove_if(
        topTierChoices.begin(),
        topTierChoices.end(),
        [](const tuple<double, double, double>& x) {
            return (std::get<2>(x) >= 0.85);
        }), topTierChoices.end());

    random_device rd;
    mt19937 g(rd());

    if (topTierChoices.size() > 0) {
        shuffle(topTierChoices.begin(), topTierChoices.end(), g);
        auto selection = topTierChoices[0];
        this->x = get<0>(selection);
        this->y = get<1>(selection);
    } else if (lessUniformChoices.size() > 0) {
        shuffle(lessUniformChoices.begin(), lessUniformChoices.end(), g);
        auto selection = lessUniformChoices[0];
        this->x = get<0>(selection);
        this->y = get<1>(selection);
    } else {
        shuffle(choices.begin(), choices.end(), g);
        auto selection = choices[0];
        this->x = get<0>(selection);
        this->y = get<1>(selection);
    }
}