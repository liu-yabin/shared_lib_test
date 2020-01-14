/*! @file my_lib_export.h
@author liu yabin
@date 2020/01/14
@copydoc copyright
@brief Define export macro.
*/

#pragma once

#ifdef MY_LIB_SHARED_BUILD
#ifdef _WIN32
#ifdef MY_LIB_EXPORT
#define MY_LIB_LIB_DECL __declspec(dllexport)
#else
#define MY_LIB_LIB_DECL __declspec(dllimport)
#endif  // MY_LIB_EXPORT
#else
#define MY_LIB_LIB_DECL
#endif  // _WIN32
#else
#define MY_LIB_LIB_DECL
#endif  // MY_LIB_SHARED_BUILD
