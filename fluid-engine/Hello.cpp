#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>
#include <thread>
using namespace std;
using namespace chrono;

const int kBufferSize = 80;
const char* kGrayScaleTable = " .:-=+*#%@";
const size_t kGrayScaleTableSize = sizeof(kGrayScaleTable) / sizeof(char);

// x는 위치값으로 0과 1사이의 값을 가져 이를 버퍼 사이즈로 키워 위치를 표현한다

void updateWave(const double timeInterval, double* x, double* speed) {

	// 위치 업데이트
	(*x) += timeInterval * *speed;

	// x가 1.0을 넘어가거나 0.0아래로 내려가는 경우 벽에 닿은 것으로 속도와 위치값을 반대로 돌림
	if ((*x) > 1.0) {
		(*speed) *= -1.0;
		(*x) = 1.0 + timeInterval * (*speed);
	}
	else if ((*x) < 0.0) {
		(*speed) *= -1.0;
		(*x) = timeInterval * (*speed);

	}
}

void accumulateWaveToHeightField(
	const double x,
	const double waveLength,
	const double maxHeight,
	array<double, kBufferSize>* heightfield) 
{
	const double quarterWaveLength = 0.25 * waveLength;
	
	const int start = static_cast<int>((x - quarterWaveLength) * kBufferSize);
	const int end = static_cast<int>((x + quarterWaveLength) * kBufferSize);

	for (int i = start; i <= end; i++) {
		int iNew = i;
		if (i < 0) {
			iNew = -i - 1;
		}
		else if (i >= static_cast<int>(kBufferSize)) {
			iNew = 2 * kBufferSize - i - 1;
		}

		double distance = fabs((i + 0.5) / kBufferSize - x);
		double height = maxHeight * 0.5
			* (cos(min(distance * M_PI / quarterWaveLength, M_PI)) + 1.0);
		(*heightfield)[iNew] += height;

	}

}

void draw(
	const array<double, kBufferSize>& heightField) {
	string buffer(kBufferSize, ' ');
	for (size_t i = 0; i < kBufferSize; i++) {
		double height = heightField[i];
		size_t tableIndex = min(
			static_cast<size_t>(floor(kGrayScaleTableSize * height)),
			kGrayScaleTableSize - 1
		);

		buffer[i] = kGrayScaleTable[tableIndex];
	}

	for (size_t i = 0; i < kBufferSize; i++) {
		printf("\b");
	}

	printf("%s", buffer.c_str());
	fflush(stdout);
}

int main() {
	const double waveLengthX = 0.8;
	const double waveLengthY = 1.2;

	const double maxHeightX = 0.5;
	const double maxHeightY = 0.4;

	double x = 0.0;
	double y = 1.0;

	double speedX = 1.0;
	double speedY = -0.5;

	const int fps = 100;
	const double timeInterval = 1.0 / fps;
	array<double, kBufferSize> heightField;

	for (int i = 0; i < 1000; i++) {
		updateWave(timeInterval, &x, &speedX);
		updateWave(timeInterval, &y, &speedY);

		for (double& height : heightField) {
			height = 0.0;
		}

		accumulateWaveToHeightField(
			x, waveLengthX, maxHeightX, &heightField
		);
		accumulateWaveToHeightField(
			y, waveLengthY, maxHeightY, &heightField
		);

		draw(heightField);

		this_thread::sleep_for(milliseconds(1000 / fps));
		
	}

	printf("\n");
	fflush(stdout);

	return 0;

}

