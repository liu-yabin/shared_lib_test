/*! @file my_lib.h
@author liu yabin
@date 2020/01/14
@copydoc copyright
@brief test for shared and static library.
*/

#pragma once

#include <iostream>

#include "my_lib/my_lib_export.h"

class MY_LIB_API MyClass {
public:
    static void MyPrint();
};
