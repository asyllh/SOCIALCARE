/*--------------/
EA_ALG
consts.h
UoS
27/01/2022
/--------------*/

#ifndef EA_ALG_CONSTS_H
#define EA_ALG_CONSTS_H

#include <iostream>
#include <vector>
#include <set>
#include <cmath>
#include <climits>
#include <cstring>
#include <algorithm> // For std::sort, std::min_element
#include <string> // For std::to_string in Visual Studio

#include <cstdio>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
// #  ifdef MODULE_API_EXPORTS
#    define MODULE_API __declspec(dllexport)
// #  else
// #    define MODULE_API __declspec(dllimport)
// #  endif
#else
#  define MODULE_API
#endif

#define bigM 9999999;

#endif //EA_ALG_CONSTS_H
