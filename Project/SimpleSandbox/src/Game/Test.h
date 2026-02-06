#include "EnginePCH.h"
#include "Math/Vector.h"
using namespace Umbra::Math;

void TestVec()
{
    FVector2D testVecF1 = FVector2D(30.5f, 50.5f);
    FVector2D testVec2 = FVector2D(30, 50);
    IntVector2D testVecI3 = IntVector2D(61, 10);
    FVector2D testVecF3 = FVector2D(testVecI3);
    hlslpp::float2 vf = (10, 2.5f);
    // FVector2D testVecF1 = (10, 2.5f);
    //---

    int a = 5;
    testVecF1 * 1;
    5 * testVecF1;
    testVecF1 * 10.5f;
    2.5f * testVecF1;
    testVecF1 *= 4;
    testVecF1 *= 22.4;
    testVecF1 / 6;
    testVecF1 / 10.5f;
    testVecF1 /= 0.5f;
    testVecF1 /= 112;
    if (testVecF1 == testVec2)
    {
    }
    if (testVecF1 != testVec2)
    {
    }
    testVecF1.Magnitude();
    testVecF1 + testVec2;
    testVecF1 += testVec2;
    testVecF1 - testVec2;
    testVecF1 -= testVec2;

    // testVecF1 - testVecI3;
    //     testVecF1 - testVecI3;
    //   testVecF1 -= testVec2;

    5 * testVecI3;
    testVecI3 * 10;
    testVecI3 * 1.50f;
    testVecI3 *= 11;
    testVecI3 /= 11;
    // testVecF1 * 10.5f;
    // 2.5f * testVecF1;
    // testVecF1 *= 4;
    // testVecF1 *= 22.4;
    // testVecF1 / 6;
    // testVecF1 / 10.5f;
    // testVecF1 /= 0.5f;
    // testVecF1 /= 112;
    //---
    // testVec2 * 1;
    // 5 * testVec2;
    // testVec2 * 10.5f;
    // 2.5f * testVec2;
    // float Mag = testVec1.Magnitude();
};