#include "mandelbrot.hpp"
#include "GUI_Paint.h"
#include <random>
#include <thread>
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
#include <iostream>

void MandelbrotSet::SetRender(UBYTE * image) {
	rendered = image;
}

void MandelbrotSet::InitMandelbrotSet() {
	w = 4.0;
	h = 2.0;
	if(renderedResX > 0 && renderedResY > 0) {
		double aspectRatio = (double) renderedResX / (double) renderedResY;
		h = w / aspectRatio;
	}
	x = -1.0;
	y = 0.0;
	renderedResX = 0;
	renderedResY = 0;
	srand(time(0));
}
bool MandelbrotSet::IsMandelPoint(double fX, double fY, int iterations) {
	double z_x = fX;
	double z_y = fY;
	for(int i = 0; i < iterations; ++i) {
		double z_x_old = z_x;
		z_x = z_x * z_x - z_y * z_y + fX;
		z_y = 2.0 * z_x_old * z_y + fY;
		if(z_x * z_x + z_y * z_y > 4.0) {
			return false;
		}
	}
	return true;
}

void MandelbrotSet::Render(UWORD xResolution, UWORD yResolution) {
	const int numThreads = 4;
	int chunkSize = yResolution / numThreads;
	std::vector < std::thread > threads;
	std::mutex mutex;
	static int imageIndex = 0;
	bool validImage = false;
	int blackPixelCount = 0;
	int totalPixelCount = xResolution * yResolution;
	int retryCount = 0;
	const int maxRetries = 20;
	const int minBlackPixelCount = totalPixelCount * 0.2;
	const int maxBlackPixelCount = totalPixelCount * 0.9;

	double aspectRatio = (double) xResolution / (double) yResolution;

	int iter = (50 + std::max(0.0, -log10(w)) * 100); 
	iter += rand() % 50; 
	auto renderChunk = [ & ](int startY, int endY, int & localBlackPixelCount) {
		for(int i = startY; i < endY; ++i) {
			for(int j = 0; j < xResolution; ++j) {
				double p_x = this - > x - this - > w / 2.0 + (double) j / (double) xResolution * this - > w;
				double p_y = this - > y - this - > h / 2.0 + (double) i / (double) yResolution * this - > h;
				bool isMandelPoint = IsMandelPoint(p_x, p_y, iter);
				if(isMandelPoint) {
					localBlackPixelCount++;
				}
				Paint_SetPixel(j, i, isMandelPoint ? BLACK : WHITE);
			}
		}
	};
	while(!validImage) {
		blackPixelCount = 0;

		if(imageIndex == 0) {
			if(w / h != aspectRatio) {
				h = w / aspectRatio;
			}
		} else {
			ZoomOnInterestingArea();
			h = w / aspectRatio; 
		}
		imageIndex++;
		std::vector < int > localBlackCounts(numThreads, 0);
		for(int t = 0; t < numThreads; ++t) {
			int startY = t * chunkSize;
			int endY = (t == numThreads - 1) ? yResolution : (t + 1) * chunkSize;
			threads.emplace_back(renderChunk, startY, endY, std::ref(localBlackCounts[t]));
		}
		for(auto & thread: threads) {
			if(thread.joinable()) {
				thread.join();
			}
		}
		for(int count: localBlackCounts) {
			blackPixelCount += count;
		}
		if(blackPixelCount >= minBlackPixelCount && blackPixelCount <= maxBlackPixelCount) {
			validImage = true;
		} else {
			retryCount++;
			if(retryCount >= maxRetries) {
				std::cout << "Max retries reached. Exploring a new random region." << std::endl;
				x = (rand() % 10000 - 5000) / 1000.0;
				y = (rand() % 8000 - 4000) / 1000.0;
				w = std::min(1.5, w); 
				h = w / aspectRatio;
				retryCount = 0;
			} else {

				x += ((rand() % 1000) - 500) / 100.0; 
				y += ((rand() % 1000) - 500) / 100.0;
				double zoomFactor = 1.0 + ((rand() % 200) / 100.0); 
				w *= zoomFactor;
				h = w / aspectRatio; 
				if(w > 1.0) w = 1.0;
				if(w < 0.05) w = 0.05;
				std::cout << "Exploring new region: retry " << retryCount << " with zoom factor " << zoomFactor << std::endl;
			}
		}
		threads.clear();
	}
}
double MandelbrotSet::GetImprovedUniformnessOfArea(double fW, double fH, int xOffset, int yOffset, int wDiv, int hDiv) {
	unsigned long long numWhite = 0;
	unsigned long long numBlack = 0;
	double totalPixels = fW * fH;
	for(int wStart = 0; wStart < fW; ++wStart) {
		for(int hStart = 0; hStart < fH; ++hStart) {
			int xPointIndex = xOffset + wStart;
			int yPointIndex = yOffset + hStart;
			if(Paint_GetPixel(xPointIndex, yPointIndex) == WHITE) numWhite++;
			else numBlack++;
		}
	}

	return std::max((double) numWhite / totalPixels, (double) numBlack / totalPixels);
}

void MandelbrotSet::ZoomOnInterestingArea() {
	std::vector < std::tuple < double, double, double >> choices;

	auto uniformnessTopLeft = GetImprovedUniformnessOfArea(this - > renderedResX / 2, this - > renderedResY / 2, 0, 0, 2, 2);
	choices.emplace_back(this - > x - this - > w / 4, this - > y + this - > h / 4, uniformnessTopLeft);
	auto uniformnessTopRight = GetImprovedUniformnessOfArea(this - > renderedResX / 2, this - > renderedResY / 2, this - > renderedResX / 2, 0, 2, 2);
	choices.emplace_back(this - > x + this - > w / 4, this - > y + this - > h / 4, uniformnessTopRight);
	auto uniformnessBottomLeft = GetImprovedUniformnessOfArea(this - > renderedResX / 2, this - > renderedResY / 2, 0, this - > renderedResY / 2, 2, 2);
	choices.emplace_back(this - > x - this - > w / 4, this - > y - this - > h / 4, uniformnessBottomLeft);
	auto uniformnessBottomRight = GetImprovedUniformnessOfArea(this - > renderedResX / 2, this - > renderedResY / 2, this - > renderedResX / 2, this - > renderedResY / 2, 2, 2);
	choices.emplace_back(this - > x + this - > w / 4, this - > y - this - > h / 4, uniformnessBottomRight);

	w /= 2.0;
	h /= 2.0;

	choices.erase(std::remove_if(choices.begin(), choices.end(),
		[](const std::tuple < double, double, double > & region) {
			return std::get < 2 > (region) >= 0.98; 
		}), choices.end());

	choices.erase(std::remove_if(choices.begin(), choices.end(),
		[](const std::tuple < double, double, double > & region) {
			return std::get < 2 > (region) <= 0.35; 
		}), choices.end());

	std::random_device rd;
	std::mt19937 g(rd());
	if(!choices.empty()) {
		std::shuffle(choices.begin(), choices.end(), g);
		auto[newX, newY, _] = choices.front();
		x = newX;
		y = newY;
	} else {

		x += ((rand() % 100) - 50) / 1000.0;
		y += ((rand() % 100) - 50) / 1000.0;
	}
}

void explore_branch_like_areas(double & zoom_factor, double & pan_x, double & pan_y) {

	if(zoom_factor > 1000.0) {

		pan_x += 0.01 * ((rand() % 2 == 0) ? 1 : -1);
		pan_y += 0.01 * ((rand() % 2 == 0) ? 1 : -1);
	} else {

		zoom_factor *= 0.9;
	}
}

int max_iterations(double zoom_level, double escape_time_gradient) {
	if(zoom_level > 800.0) {

		if(escape_time_gradient > 0.5) { 
			return 1500; 
		}
		return 1000; 
	}

	if(zoom_level < 100.0 && escape_time_gradient < 0.1) {
		return 100; 
	}
	return 500; 
}
