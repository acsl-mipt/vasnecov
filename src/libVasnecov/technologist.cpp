/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "technologist.h"
#include <iostream>
#include <sstream>
#include "bmcl/Logging.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

int Vasnecov::panic(const GLstring &problemText1, const GLstring &problemText2)
{
    BMCL_CRITICAL() << "3D: " << problemText1 << problemText2;
    return 1;
}


int Vasnecov::panic(const GLstring &problemText1, int errorCode)
{
    BMCL_CRITICAL() << "3D: "  << problemText1 << errorCode;
    return 1;
}


int Vasnecov::panic(const GLstring &problemText1, float value)
{
    BMCL_CRITICAL() << "3D: "  << problemText1 << value;
    return 1;
}


int Vasnecov::problem(const GLstring &problemText1, const GLstring &problemText2)
{
    BMCL_WARNING() << "3D: "  << problemText1 << problemText2;
    return 1;
}


int Vasnecov::problem(const GLstring &problemText1, int errorCode)
{
    BMCL_WARNING() << "3D: "  << problemText1 << errorCode;
    return 1;
}


int Vasnecov::problem(const GLstring &problemText1, float value)
{
    BMCL_WARNING() << "3D: "  << problemText1 << value;
    return 1;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
