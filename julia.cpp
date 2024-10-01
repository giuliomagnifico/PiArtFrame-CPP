#include "julia.hpp"
#include <complex>
#include <random>
#include <algorithm>
#include <ctime>
#include <vector>
#include <tuple>
#include <cmath>
#include "GUI_Paint.h"
#include <thread>  
#include <mutex>   
#include <stdexcept> 
#include <iostream> 

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
    const int numThreads = 4;
    int chunkSize = yResolution / numThreads;
    std::vector<std::thread> threads;
    std::mutex mutex;

    static int imageIndex = 0;
    bool validImage = false;

    int blackPixelCount = 0;
    int totalPixelCount = xResolution * yResolution;
    int retryCount = 0;
    const int maxRetries = 1000;
    const int minBlackPixelCount = totalPixelCount * 0.20;  // Minimum 20% black pixels
    const int maxBlackPixelCount = totalPixelCount * 0.80;  // Maximum 80% black pixels

    auto renderChunk = [&](int startY, int endY, int& localBlackPixelCount) {
        for (int i = startY; i < endY; ++i) {
            for (int j = 0; j < xResolution; ++j) {
                double p_x = this->x - this->w / 2.0 + (double)j / (double)xResolution * this->w;
                double p_y = this->y - this->h / 2.0 + (double)(i + 1) / (double)yResolution * this->h;
                bool isJuliaPoint = IsJuliaPoint(p_x, p_y, 30 + std::max(0.0, -log10(w)) * 100);

                if (isJuliaPoint) {
                    localBlackPixelCount++;  // Count black pixels in this chunk
                }

                Paint_SetPixel(j, i, isJuliaPoint ? BLACK : WHITE);
            }
        }
    };

    while (!validImage) {
        blackPixelCount = 0;

        double aspectRatio = (double)xResolution / (double)yResolution;

        if (imageIndex == 0) {
            // Initial aspect ratio adjustment
            if (w / h != aspectRatio) {
                h = w / aspectRatio;
            }
        } else {
            // Pan and zoom with each new image
            x += ((rand() % 100) - 50) / 500.0;
            y += ((rand() % 100) - 50) / 500.0;
            w *= 0.95;  // Less aggressive zoom
            h = w / aspectRatio;
            x = std::min(std::max(x, -1.5), -0.5);
            y = std::min(std::max(y, -0.5), 0.5);
        }

        imageIndex++;

        std::vector<int> localBlackCounts(numThreads, 0);  // Black pixel counts per thread
        for (int t = 0; t < numThreads; ++t) {
            int startY = t * chunkSize;
            int endY = (t == numThreads - 1) ? yResolution : (t + 1) * chunkSize;
            threads.emplace_back(renderChunk, startY, endY, std::ref(localBlackCounts[t]));
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Sum the black pixel counts from all threads
        for (int count : localBlackCounts) {
            blackPixelCount += count;
        }

        if (blackPixelCount >= minBlackPixelCount && blackPixelCount <= maxBlackPixelCount) {
            validImage = true;  // Image is valid, exit the loop
        } else {
            retryCount++;
            if (retryCount >= maxRetries) {
                std::cout << "Max retries reached. Resetting zoom." << std::endl;
                InitJuliaSet();  // Reset zoom and position
                retryCount = 0;
            } else if (blackPixelCount == 0) {
                std::cout << "No black pixels (0%), exploring new region..." << std::endl;
                x += ((rand() % 100) - 50) / 50.0;  // Larger jump
                y += ((rand() % 100) - 50) / 50.0;
                w *= 1.5;  // Zoom out more to explore larger areas
                h = w / aspectRatio;
            } else if (blackPixelCount < minBlackPixelCount) {
                std::cout << "Too few black pixels (" << (double)blackPixelCount / totalPixelCount * 100 << "%), regenerating..." << std::endl;
                if (retryCount % 30 == 0) {
                    // Explore new region every 10 retries
                    x += ((rand() % 100) - 50) / 100.0;
                    y += ((rand() % 100) - 50) / 100.0;
                    w *= 1.2;
                    h = w / aspectRatio;
                    x = std::min(std::max(x, -1.5), -0.5);
                    y = std::min(std::max(y, -0.5), 0.5);
                }
            } else {
                std::cout << "Too many black pixels (" << (double)blackPixelCount / totalPixelCount * 100 << "%), regenerating..." << std::endl;
            }
        }
    }
}

bool JuliaSet::IsJuliaPoint(double fX, double fY, int iterations)
{
    std::complex<double> z(fX, fY);
    std::complex<double> c(-0.8, 0.156);  

    for (int i = 0; i < iterations; ++i) {
        z = z * z + c;
        if (std::abs(z) > 2.0) {
            return false;
        }
    }
    return true;
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
