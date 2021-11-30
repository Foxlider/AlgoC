#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float minV(int argc, char **argv)
{
    float minValue = 100000000000000000;
    for (int i = 2; i < argc; i++) {
        if (atof(argv[i]) < minValue)
            minValue = atof(argv[i]);
    }
    return minValue;
}

float maxV(int argc, char **argv)
{
    float maxValue = -100000000000000000;
    for (int i = 2; i < argc; i++) {
        if (atof(argv[i]) > maxValue)
            maxValue = atof(argv[i]);
    }
    return maxValue;
}

float moy(int argc, char **argv)
{
    float moyenne = 0.0;
    for (int i = 2; i < argc; i++) {
        moyenne += atof(argv[i]);
    }
    moyenne /= (argc - 2);
    return moyenne;
}

float ecartType(int argc, char **argv)
{
    float ecartType = 0.0;
    float variance = 0.0;
    float moyenne = moy(argc, argv);
    float stock = 0;
    for (int i = 2; i < argc; i++) {
        stock = (atof(argv[i]) - moyenne) * (atof(argv[i]) - moyenne);
        variance += stock;
    }
    variance /= (argc - 2);
    if (variance >= 0)
        ecartType = sqrt(variance);
    else
        return (EXIT_FAILURE);
    return ecartType;
}