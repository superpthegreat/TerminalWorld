
// Controls: A = Turn Left, D = Turn Right, W = Walk Forwards, S = Walk Backwards

// Need to use external console instead of VScode integrated terminal

// vs code shortcut to open external console ctrl + shift + c

// how to compile with g++
// g++ -o terminalWorld.exe terminalWorld.cpp

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <math.h>
#include <time.h>


using namespace std;

#define UNICODE         // so unicode works
#define _UNICODE        // so unicode works

#include <stdio.h>
#include <Windows.h>


int nScreenWidth = 120;         // console screen size X (columns)
int nScreenHeight = 40;         // console screen size Y (rows)
int nMapWidth = 16;             // world dimensions
int nMapHeight = 16;

float fPlayerX = 14.7f;         // player start position
float fPlayerY = 5.09f;
float fPlayerA = 0.0f;          // player start rotation
float fFOV = 3.14159f / 4.0f;   // field of view
float fDepth = 16.0f;           // maximum rendering distance
float fSpeed = 5.0f;            // walking Speed

int main()
{
    srand(time(NULL)); // Randomize seed initialization

    // Create Screen Buffer
    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;
    

    // Create Map of world space # = wall block, . = space
    wstring map;
    wstring FullString;

    // very top of map
    map += L"################";

    // random generate a map
    // by creating a random string of # or .

    // flag to decide whether a row should be empty
    bool skipTurn = true;
    

    for(int i=0; i < 14; i++){

        int leftRight = rand() % 2;             // 0 = left, right = 1
        int amount = rand() % 13 + 1;           // random number between 1 and n, for amount of #
        int leftOver = nMapWidth - amount;      // number of . to fill in
        int randomSpace = rand() % 2;           // choose whether a solid wall will have a random gap
        int randomLetter = rand() % 16 + 1;     // where in that wall the gap will be

        //empty line, every other line
        if(skipTurn){
            FullString += L"#..............#";
            skipTurn = false;
            map += FullString;
        }
        else {
            // filling out from left side
            if (leftRight == 0)
            {
                FullString += L"#";
                for (int j = 0; j < amount - 1; j++)
                {   
                    if(randomSpace == 1 && randomLetter == j){
                        FullString += L".";
                    }
                    else{
                        FullString += L"#";
                    }
                    
                }

                for (int k = 0; k < leftOver - 1; k++)
                {
                    FullString += L".";
                }

                FullString += L"#";
            }

            // filling out from right side
            if (leftRight == 1)
            {
                FullString += L"#";
                for (int k = 0; k < leftOver - 1; k++)
                {
                    FullString += L".";
                }

                for (int j = 0; j < amount - 1; j++)
                {
                    if (randomSpace == 1 && randomLetter == j)
                    {
                        FullString += L".";
                    }
                    else
                    {
                        FullString += L"#";
                    }
                }

                FullString += L"#";
            }

            map += FullString;
            skipTurn = true;
        }

        FullString.erase();

    }

    
    

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while (1)
    {
        // time differential per frame to calculate modification to movement speeds,
        // to ensure consistant movement, as ray-tracing is non-deterministic
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // handle CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;

        // handle CW Rotation
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (fSpeed * 0.75f) * fElapsedTime;

        // handle forwards movement & collision
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
                ;
                fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
                ;
            }
        }

        // handle backwards movement & collision
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            fPlayerY -= cosf(fPlayerA) * fSpeed * fElapsedTime;
            ;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX += sinf(fPlayerA) * fSpeed * fElapsedTime;
                ;
                fPlayerY += cosf(fPlayerA) * fSpeed * fElapsedTime;
                ;
            }
        }

        for (int x = 0; x < nScreenWidth; x++)
        {
            // for each column, calculate the projected ray angle into world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            // find distance to wall
            float fStepSize = 0.1f;       // increment size for ray casting, decrease to increase
            float fDistanceToWall = 0.0f; //                                      resolution

            bool bHitWall = false;  // set when ray hits wall block
            bool bBoundary = false; // set when ray hits boundary between two wall blocks

            float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
            float fEyeY = cosf(fRayAngle);

            // incrementally cast ray from player, along ray angle, testing for
            // intersection with a block
            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += fStepSize;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                // test if ray is out of bounds
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true; // just set distance to maximum depth
                    fDistanceToWall = fDepth;
                }
                else
                {
                    // ray is inbounds so test to see if the ray cell is a wall block
                    if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
                    {
                        // ray has hit wall
                        bHitWall = true;

                        // To highlight tile boundaries, cast a ray from each corner
                        // of the tile, to the player. The more coincident this ray
                        // is to the rendering ray, the closer we are to a tile boundary
                        vector<pair<float, float>> p;

                        // Test each corner of hit tile, storing the distance from
                        // the player, and the calculated dot product of the two rays
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                // Angle of corner to eye
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        // sort Pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right)
                             { return left.first < right.first; });

                        // first two/three are closest (we will never see all four)
                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound)
                            bBoundary = true;
                        if (acos(p.at(1).second) < fBound)
                            bBoundary = true;
                        if (acos(p.at(2).second) < fBound)
                            bBoundary = true;
                    }
                }
            }

            // calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            // shader walls based on distance
            short nShade = ' ';
            if (fDistanceToWall <= fDepth / 4.0f)
                nShade = 0x2588; // Very close
            else if (fDistanceToWall < fDepth / 3.0f)
                nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)
                nShade = 0x2592;
            else if (fDistanceToWall < fDepth)
                nShade = 0x2591;
            else
                nShade = ' '; // Too far away

            if (bBoundary)
                nShade = ' '; // Black it out

            for (int y = 0; y < nScreenHeight; y++)
            {
                // Each Row
                if (y <= nCeiling)
                    screen[y * nScreenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else // Floor
                {
                    // Shade floor based on distance
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25)
                        nShade = '#';
                    else if (b < 0.5)
                        nShade = 'x';
                    else if (b < 0.75)
                        nShade = '.';
                    else if (b < 0.9)
                        nShade = '-';
                    else
                        nShade = ' ';
                    screen[y * nScreenWidth + x] = nShade;
                }
            }
        }

        // display stats
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        // display map
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

        // display frame
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
    }

    return 0;
}

