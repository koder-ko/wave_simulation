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


// 밑으로 내려서 main부터 읽어 책 순서대로 읽으면 됨

// 화면(문자열) 너비
const int kBufferSize = 80;

const char* kGrayScaleTable = " .:-=+*#%@";
const size_t kGrayScaleTableSize = sizeof(kGrayScaleTable) / sizeof(char);

// 파동 이동
void updateWave(const double timeInterval, double* x, double* speed) {

	// 위치 업데이트 시간 간격 * 속도
	// x는 위치값으로 0과 1사이의 값을 가져 이를 문자열 너비로 키워 위치를 표현한다
	(*x) += timeInterval * *speed;

	// x가 1.0을 넘어가거나 0.0아래로 내려가는 경우 벽에 닿은 것으로 속도와 위치값을 반대로 돌림
	if ((*x) > 1.0) {
		// -1곱해서 방향 반대로
		(*speed) *= -1.0;
		(*x) = 1.0 + timeInterval * (*speed);
	}
	else if ((*x) < 0.0) {
		(*speed) *= -1.0;
		(*x) = timeInterval * (*speed);

	}
}


// 파동 위치에 더해서 표현
void accumulateWaveToHeightField(
	const double x,
	const double waveLength,
	const double maxHeight,
	array<double, kBufferSize>* heightfield) 
{
	const double quarterWaveLength = 0.25 * waveLength;


	// 파동 중심점 위치(double x)에서 오른쪽 왼쪽으로(-, + quarterWaveLength) 시작과 끝의 위치를 구한다
	const int start = static_cast<int>((x - quarterWaveLength) * kBufferSize);
	const int end = static_cast<int>((x + quarterWaveLength) * kBufferSize);


	// 위치에 대해 시작과 끝까지 1씩 더해가면서 동작
	for (int i = start; i <= end; i++) {

		// 만약 벽에 닿았으면 그게 튕겨서 원래 오던 파동과 겹치게 되므로 그거 계산
		int iNew = i;
		if (i < 0) {
			iNew = -i - 1;
		}
		else if (i >= static_cast<int>(kBufferSize)) {
			iNew = 2 * kBufferSize - i - 1;
		}

		// 높이를 중심점에서 떨어진 거리에 대해서 cos으로 계산
		double distance = fabs((i + 0.5) / kBufferSize - x);
		double height = maxHeight * 0.5
			* (cos(min(distance * M_PI / quarterWaveLength, M_PI)) + 1.0);
		(*heightfield)[iNew] += height;

	}

}


// 그리기(애니메이션)
void draw(
	const array<double, kBufferSize>& heightField) {

	// 출력될 문자열(buffer)
	string buffer(kBufferSize, ' ');

	// 문자열 각 자리당
	for (size_t i = 0; i < kBufferSize; i++) {

		// 높이 가져옴
		double height = heightField[i];

		// 높이에 해당하는 grayscale(빽빽함으로 명암표현, 여기선 빽빽할수록 높은거)로 변환
		size_t tableIndex = min(
			static_cast<size_t>(floor(kGrayScaleTableSize * height)),
			kGrayScaleTableSize - 1
		);

		// 문자열에 직접 집어넣기
		buffer[i] = kGrayScaleTable[tableIndex];
	}

	// 넘기기
	for (size_t i = 0; i < kBufferSize; i++) {
		printf("\b");
	}

	printf("%s", buffer.c_str());

	// 흘려보내기
	fflush(stdout);
}

// 실행단
int main() {

	// 차례대로 파동의 길이, 최대높이, 중심점 위치, 속도 지정
	const double waveLengthX = 0.8;
	const double waveLengthY = 1.2;

	const double maxHeightX = 0.5;
	const double maxHeightY = 0.4;

	double x = 0.0;
	double y = 1.0;

	double speedX = 1.0;
	double speedY = -0.5;

	// 애니메이션 위한 frame per second와 시간 간격 계산
	const int fps = 100;
	const double timeInterval = 1.0 / fps;

	// 높이 저장해두는 배열
	array<double, kBufferSize> heightField;

	// 0초부터 1000초까지
	for (int i = 0; i < 1000; i++) {

		// 파동 업데이트(이동)
		updateWave(timeInterval, &x, &speedX);
		updateWave(timeInterval, &y, &speedY);

		// 높이 초기화(이전 높이가 저장되어있을 수 있음)
		for (double& height : heightField) {
			height = 0.0;
		}

		// 나온 것들을 더해주기
		accumulateWaveToHeightField(
			x, waveLengthX, maxHeightX, &heightField
		);
		accumulateWaveToHeightField(
			y, waveLengthY, maxHeightY, &heightField
		);

		// 그려주기
		draw(heightField);

		// 기다리기(없으면 너무 빨리 다 지나감)
		this_thread::sleep_for(milliseconds(1000 / fps));
		
	}

	// 마무으리
	printf("\n");
	fflush(stdout);

	return 0;

}

