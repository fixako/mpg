//
// Created by fixako on 06.07.2017.
//

#ifndef MPG_DEFAUL_VALUES_HPP
#define MPG_DEFAUL_VALUES_HPP

#include <chrono>

class MpgDefaulValues
{
public:
    static std::chrono::duration<double>
            targetFrequency = std::chrono::duration<double>(1.0);
    static std::chrono::duration<double>
            targetDuration = std::chrono::duration<double>(2.0),
            clickDuration = std::chrono::duration<double>(2.0),
            missclickDuration = std::chrono::duration<double>(2.0);

    static double targetMaxSize = 0.2, clickMaxSize = 0.2, missclickMaxSize = 0.2;
    static double (*targetOpacityFunction)(double) =    [](double x) { return x; };
    static double toa = 0, tob = 1;
    static double (*targetSizeFunction)(double) =       [](double x) { return x; };
    static double tsa = 0, tsb = 1;
    static double (*clickOpacityFunction)(double) =     [](double x) { return x; };
    static double coa = 0, cob = 1;
    static double (*clickSizeFunction)(double) =        [](double x) { return x; };
    static double csa = 0, csb = 1;
    static double (*missclickOpacityFunction)(double) = [](double x) { return x; };
    static double moa = 0, mob = 1;
    static double (*missclickSizeFunction)(double) =    [](double x) { return x; };
    static double msa = 0, msb = 1;
    static bool targetOMirror = true, clickOMirror = false, missclickOMirror = false;
    static bool targetSMirror = true, clickSMirror = false, missclickSMirror = false;

    MpgDefaulValues()
    {

    }

};

#endif //MPG_DEFAUL_VALUES_HPP
