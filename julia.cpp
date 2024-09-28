#include "julia.hpp"
#include <complex>
#include <random>
#include <algorithm>
#include <ctime>
#include <vector>
#include <tuple>
#include <cmath>
#include "GUI_Paint.h"

void JuliaSet::InitJuliaSet()
{
    w = 4;
    h = 2;
    x = 0;
    y = 0;
    rendered = NULL;
    renderedResX = 0;
    renderedResY = 0;
    srand(time(0));  
}

void JuliaSet::SetRender(UBYTE* image)
{
    rendered = image;
}

void JuliaSet::Render(UWORD xResolution, UWORD yResolution)
{
    static int imageIndex = 0;
    bool validImage = false;

    while (!validImage) {
        double aspectRatio = (double)xResolution / (double)yResolution;

        if (imageIndex == 0) {
            if (w / h != aspectRatio) {
                h = w / aspectRatio;
            }
        } else if (imageIndex < 5) {
            x += ((rand() % 100) - 50) / 500.0;
            y += ((rand() % 100) - 50) / 500.0;
            w *= 1.0 + ((rand() % 40) - 20) / 100.0;
            h *= 1.0 + ((rand() % 40) - 20) / 100.0;

            w = std::min(std::max(w, 0.05), 4.0);
            h = std::min(std::max(h, 0.05), 2.0);
            h = w / aspectRatio;
            x = std::min(std::max(x, -1.5), -0.5);
            y = std::min(std::max(y, -0.5), 0.5);
        } else {
            x += ((rand() % 50) - 25) / 1000.0;
            y += ((rand() % 50) - 25) / 1000.0;
            w *= 1.0 + ((rand() % 20) - 10) / 100.0;
            h *= 1.0 + ((rand() % 20) - 10) / 100.0;

            w = std::min(std::max(w, 0.05), 4.0);
            h = std::min(std::max(h, 0.05), 2.0);
            h = w / aspectRatio;
            x = std::min(std::max(x, -1.5), -0.5);
            y = std::min(std::max(y, -0.5), 0.5);
        }

        imageIndex++;

        int iter = (50 + std::max(0.0, -log10(w)) * 100);
        iter += rand() % 50;

        std::vector<std::vector<bool>> columns;
        int whiteCount = 0;
        int blackCount = 0;

        for (int i = yResolution - 1; i >= 0; --i) {
            std::vector<bool> rows;
            for (int j = 0; j < xResolution; ++j) {
                double p_x = this->x - this->w / 2.0 + (double)j / (double)xResolution * this->w;
                double p_y = this->y - this->h / 2.0 + (double)(i + 1) / (double)yResolution * this->h;
                bool isJuliaPoint = IsJuliaPoint(p_x, p_y, iter);
                rows.emplace_back(isJuliaPoint);

                if (isJuliaPoint) {
                    blackCount++;
                } else {
                    whiteCount++;
                }
            }
            columns.emplace_back(rows);
        }

        renderedResX = xResolution;
        renderedResY = yResolution;

        double totalPixels = (double)(xResolution * yResolution);
        double blackRatio = (double)blackCount / totalPixels;
        double whiteRatio = (double)whiteCount / totalPixels;

        if (blackRatio < 0.95 && whiteRatio < 0.95) {
            validImage = true;

            for (unsigned int y = 0; y < columns.size(); ++y) {
                auto row = columns[y];
                for (unsigned int x = 0; x < row.size(); ++x) {
                    auto bitSet = row[x];
                    if (bitSet) {
                        Paint_SetPixel(x, y, BLACK);
                    } else {
                        Paint_SetPixel(x, y, WHITE);
                    }
                }
            }
        }
    }
}

bool JuliaSet::IsJuliaPoint(double fX, double fY, int iterations)
{
    std::complex<double> z(fX, fY);
    std::complex<double> c(-0.7, 0.27015);  

    for (int i = 0; i < iterations; ++i) {
        z = z * z + c;
        if (std::abs(z) > 2.0) {
            return true;
        }
    }
    return false;
}

void JuliaSet::ZoomOnInterestingArea()
{
    std::vector<std::tuple<double, double, double>> choices;

    auto uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, 0, 0, 2, 2);
    choices.emplace_back(this->x - this->w / 4, this->y + this->h / 4, uniformness);

    uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, this->renderedResX / 2, 0, 2, 2);
    choices.emplace_back(this->x + this->w / 4, this->y + this->h / 4, uniformness);

    uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, 0, this->renderedResY / 2, 2, 2);
    choices.emplace_back(this->x - this->w / 4, this->y - this->h / 4, uniformness);

    uniformness = GetImprovedUniformnessOfArea(this->renderedResX / 2, this->renderedResY / 2, this->renderedResX / 2, this->renderedResY / 2, 2, 2);
    choices.emplace_back(this->x + this->w / 4, this->y - this->h / 4, uniformness);

    w = w / 2.0;
    h = h / 2.0;

    auto lessUniformChoices = choices;
    lessUniformChoices.erase(std::remove_if(lessUniformChoices.begin(), lessUniformChoices.end(),
                                            [](const std::tuple<double, double, double>& x) { return std::get<2>(x) >= 0.95; }),
                             lessUniformChoices.end());

    auto topTierChoices = choices;
    topTierChoices.erase(std::remove_if(topTierChoices.begin(), topTierChoices.end(),
                                        [](const std::tuple<double, double, double>& x) { return std::get<2>(x) >= 0.85; }),
                         topTierChoices.end());

    std::random_device rd;
    std::mt19937 g(rd());

    if (!topTierChoices.empty()) {
        std::shuffle(topTierChoices.begin(), topTierChoices.end(), g);
        auto selection = topTierChoices[0];
        this->x = std::get<0>(selection);
        this->y = std::get<1>(selection);
    } else if (!lessUniformChoices.empty()) {
        std::shuffle(lessUniformChoices.begin(), lessUniformChoices.end(), g);
        auto selection = lessUniformChoices[0];
        this->x = std::get<0>(selection);
        this->y = std::get<1>(selection);
    } else {
        std::shuffle(choices.begin(), choices.end(), g);
        auto selection = choices[0];
        this->x = std::get<0>(selection);
        this->y = std::get<1>(selection);
    }
}

double JuliaSet::GetImprovedUniformnessOfArea(double fW, double fH, int xOffset, int yOffset, int wDiv, int hDiv)
{
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

    return std::max((double)numWhite / totalPixels, (double)numBlack / totalPixels);
}
