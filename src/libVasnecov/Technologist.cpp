/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Technologist.h"
#include <iostream>
#include <sstream>
#include <bmcl/Logging.h>

GLint Vasnecov::panic(const QString &problemText1, const QString &problemText2)
{
    BMCL_CRITICAL() << "3D: " << problemText1 << problemText2;
    return 1;
}


GLint Vasnecov::panic(const QString &problemText1, GLint errorCode)
{
    BMCL_CRITICAL() << "3D: "  << problemText1 << errorCode;
    return 1;
}


GLint Vasnecov::panic(const QString &problemText1, GLfloat value)
{
    BMCL_CRITICAL() << "3D: "  << problemText1 << value;
    return 1;
}


GLint Vasnecov::problem(const QString &problemText1, const QString &problemText2)
{
    BMCL_WARNING() << "3D: "  << problemText1 << problemText2;
    return 1;
}


GLint Vasnecov::problem(const QString &problemText1, GLint errorCode)
{
    BMCL_WARNING() << "3D: "  << problemText1 << errorCode;
    return 1;
}


GLint Vasnecov::problem(const QString &problemText1, GLfloat value)
{
    BMCL_WARNING() << "3D: "  << problemText1 << value;
    return 1;
}
