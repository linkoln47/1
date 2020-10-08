#include <iostream>
#include <chrono>
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
using namespace std;

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 1.0f;
float fPlayerY = 1.0f;
float fPlayerS = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.142857 / 4;
float fDepth = 16.0f;


int main()
{

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE , 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"##.............#";
	map += L"#####........###";
	map += L"#...#..........#";
	map += L"#...#..........#";
	map += L"#...#..........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#........#######";
	map += L"#........#.....#";
	map += L"#........#.....#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";


	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1)
	{
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
		{
			fPlayerS -= (0.8f) * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
		{
			fPlayerS += (0.8f) * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerS) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerS) * 5.0f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerS) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerS) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerS) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerS) * 5.0f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerS) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerS) * 5.0f * fElapsedTime;
			}
		}

		for (int x = 0; x < nScreenWidth; x++)
		{
			float fRayAngle = (fPlayerS - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;


			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;


			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;
						
						vector<pair<float, float>> p;

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });


						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}


			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';
			short nShadeF = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)        nShade = 0x2588;
			else if (fDistanceToWall < fDepth / 3.0f)    nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)    nShade = 0x2592;
			else if (fDistanceToWall < fDepth)           nShade = 0x2591;
			else                                         nShade = ' ';


			if (bBoundary) nShade = ' ';


			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
				{
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)                  nShadeF = '#';
					else if (b < 0.5)              nShadeF = 'x';
					else if (b < 0.75)             nShadeF = '.';
					else if (b < 0.9)              nShadeF = '-';
					else                           nShadeF = ' ';
					screen[y * nScreenWidth + x] = nShadeF;
				}
			}

		}

		//swprintf_s(screen, 40, L"X = %3.2f , Y = %3.2f , S = % 3.2f , FPS = %3.2f ", fPlayerX, fPlayerY, fPlayerS, 1.0f / fElapsedTime);

		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapHeight; ny++)
			{
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}

		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = 'P';

	screen[nScreenWidth * nScreenHeight - 1] = '\0';
	WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}