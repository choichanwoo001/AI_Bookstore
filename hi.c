#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

int main(int argc, char* argv[])
{
	//---------------------------------------------------------------------------// file 1 읽기

	BITMAPFILEHEADER bmpFile1;
	BITMAPINFOHEADER bmpInfo1;

	FILE* inputFile1 = NULL;
	inputFile1 = fopen("..\\DATA\\AICenterY.bmp", "rb");

	inputFile1 = fopen("..\\DATA\\AICenterY.bmp", "rb");
	if (inputFile1 == NULL) {
		fprintf(stderr, "File 1 could not be opened.\n");
		return 1;
	}

	fread(&bmpFile1, sizeof(BITMAPFILEHEADER), 1, inputFile1);
	fread(&bmpInfo1, sizeof(BITMAPINFOHEADER), 1, inputFile1);

	int width1 = bmpInfo1.biWidth;
	int height1 = bmpInfo1.biHeight;
	int size1 = bmpInfo1.biSizeImage;
	int bitCnt1 = bmpInfo1.biBitCount;
	int stride1 = (((bitCnt1 / 8) * width1) + 3) / 4 * 4;
	printf("W: %d(%d)\nH: %d\nS: %d\nD: %d\n\n", width1, stride1, height1, size1, bitCnt1);

	//---------------------------------------------------------------------------// file 2 읽기

	BITMAPFILEHEADER bmpFile2;
	BITMAPINFOHEADER bmpInfo2;

	FILE* inputFile2 = NULL;
	inputFile2 = fopen("..\\DATA\\AICenterY_CombinedNoise.bmp", "rb");

	inputFile2 = fopen("..\\DATA\\AICenterY_CombinedNoise.bmp", "rb");
	if (inputFile2 == NULL) {
		fprintf(stderr, "File 2 could not be opened.\n");
		return 1;
	}

	fread(&bmpFile2, sizeof(BITMAPFILEHEADER), 1, inputFile2);
	fread(&bmpInfo2, sizeof(BITMAPINFOHEADER), 1, inputFile2);

	int width2 = bmpInfo2.biWidth;
	int height2 = bmpInfo2.biHeight;
	int size2 = bmpInfo2.biSizeImage;
	int bitCnt2 = bmpInfo2.biBitCount;
	int stride2 = (((bitCnt2 / 8) * width2) + 3) / 4 * 4;
	printf("W: %d(%d)\nH: %d\nS: %d\nD: %d\n\n", width2, stride2, height2, size2, bitCnt2);

	//---------------------------------------------------------------------------//

	unsigned char* inputImg1 = NULL;
	unsigned char* inputImg2 = NULL;
	unsigned char* outputImg = NULL;
	inputImg1 = (unsigned char*)calloc(size1, sizeof(unsigned char));
	inputImg2 = (unsigned char*)calloc(size2, sizeof(unsigned char));
	outputImg = (unsigned char*)calloc(size1, sizeof(unsigned char));

	fread(inputImg1, sizeof(unsigned char), size1, inputFile1);
	fread(inputImg2, sizeof(unsigned char), size2, inputFile2);

	int** Y_1; // AICenterY
	Y_1 = (int**)calloc(sizeof(int*), height1);
	for (int i = 0; i < height1; i++)
		Y_1[i] = (int*)calloc(sizeof(int), width1);

	int** Y_2; // AICenterY_NoiseCombined
	Y_2 = (int**)calloc(sizeof(int*), height2);
	for (int i = 0; i < height2; i++)
		Y_2[i] = (int*)calloc(sizeof(int), width2);

	int** Y_3; // median 필터 처리 
	Y_3 = (int**)calloc(sizeof(int*), height1);
	for (int i = 0; i < height1; i++)
		Y_3[i] = (int*)calloc(sizeof(int), width1);

	int** Y_4; // median & mean 필터 처리
	Y_4 = (int**)calloc(sizeof(int*), height1);
	for (int i = 0; i < height1; i++)
		Y_4[i] = (int*)calloc(sizeof(int), width1);

	//---------------------------------------------------------------------------//

	double mse = 0, psnr;
	//입력(반환)을 수행할 반복문
	for (int i = 0; i < height1; i++)
	{
		for (int j = 0; j < width1; j++)
		{
			Y_1[i][j] = inputImg1[i * stride1 + 3 * j + 0];
			Y_2[i][j] = inputImg2[i * stride2 + 3 * j + 0];
			Y_3[i][j] = inputImg2[i * stride1 + 3 * j + 0]; //하는 이유는 경계값들 때문 // border 처리는 아직 
			Y_4[i][j] = inputImg2[i * stride1 + 3 * j + 0]; //하는 이유는 경계값들 때문 // border 처리는 아직 

			mse += (double)((Y_2[i][j] - Y_1[i][j]) * (Y_2[i][j] - Y_1[i][j]));
		}
	}

	mse /= (width1 * height1);
	psnr = mse != 0.0 ? 10.0 * log10(255 * 255 / mse) : 99.99;

	printf("MSE = %.2lf\nPSNR(Original) = %.2lf dB\n\n", mse, psnr); //잡음 처리가 되지 않은 PSNR

	//---------------------------------------------------------------------------//

	//--------Median--------//
	int filter[9] = { 0 }; //중앙값을 찾기 위한 작업
	int cnt = 0;
	int l = 0, r = 0; // l == i // r == j 

	for (int i = 0; i < height1 - 3; i++) //큰틀 
	{
		for (int j = 0; j < width1 - 3; j++)
		{
			for (int n = i; n < i + 3; n++) //9개씩 더하는 반복문 (우선 끝까지 가진 말자) 
			{
				for (int m = j; m < j + 3; m++)
				{
					filter[cnt] = Y_2[n][m];
					cnt++;
				}
			}

			for (int p = 0; p < cnt; p++) //9개씩 더하는 반복문
			{
				for (int q = p + 1; q < cnt; q++)
				{
					if (filter[p] < filter[q])
					{
						int tmp = 0;
						tmp = filter[p];
						filter[p] = filter[q];
						filter[q] = tmp;
					}
				}
			}

			l = i + 1; r = j + 1;
			Y_3[l][r] = filter[cnt / 2];
			cnt = 0;
		}
	}

	double mse1 = 0, psnr1;
	for (int i = 0; i < height1; i++)
	{
		for (int j = 0; j < width1; j++)
		{
			mse1 += (double)((Y_3[i][j] - Y_1[i][j]) * (Y_3[i][j] - Y_1[i][j]));
		}
	}

	mse1 /= (width1 * height1);
	psnr1 = mse1 != 0.0 ? 10.0 * log10(255 * 255 / mse1) : 99.99;

	printf("MSE = %.2lf\nPSNR(Median) = %.2lf dB\n\n", mse1, psnr1); // median 잡음처리

	//---------------------------------------------------------------------------//

	//출력을 위한 반복문
	for (int i = 0; i < height1; i++)
	{
		for (int j = 0; j < width1; j++)
		{
			outputImg[i * stride1 + 3 * j + 0] = (unsigned char)(Y_4[i][j] > 255 ? 255 : (Y_4[i][j] < 0 ? 0 : Y_4[i][j]));
			outputImg[i * stride1 + 3 * j + 1] = (unsigned char)(Y_4[i][j] > 255 ? 255 : (Y_4[i][j] < 0 ? 0 : Y_4[i][j]));
			outputImg[i * stride1 + 3 * j + 2] = (unsigned char)(Y_4[i][j] > 255 ? 255 : (Y_4[i][j] < 0 ? 0 : Y_4[i][j]));
		}
	}

	//---------------------------------------------------------------------------//

	FILE* outputFile = fopen("..\\DATA\\Output____1.bmp", "wb");

	fwrite(&bmpFile1, sizeof(BITMAPFILEHEADER), 1, outputFile);
	fwrite(&bmpInfo1, sizeof(BITMAPINFOHEADER), 1, outputFile);
	fwrite(outputImg, sizeof(unsigned char), size1, outputFile);

	//헤제
	for (int i = 0; i < height1; i++) { free(Y_1[i]); }
	free(Y_1);

	for (int i = 0; i < height2; i++) { free(Y_2[i]); }
	free(Y_2);

	for (int i = 0; i < height1; i++) { free(Y_3[i]); }
	free(Y_3);

	for (int i = 0; i < height1; i++) { free(Y_4[i]); }
	free(Y_4);

	free(outputImg);
	free(inputImg1);
	free(inputImg2);

	fclose(inputFile1);
	fclose(inputFile2);
	fclose(outputFile);

	return 0;
}