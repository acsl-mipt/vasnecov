/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VASNECOV_TEHNOLOGIST_H
#define VASNECOV_TEHNOLOGIST_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include "types.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

namespace Vasnecov
{
    GLint panic(const std::string &problemText1, const std::string &problemText2 = ""); // Функция сообщения и архивирования критических ошибок
    GLint panic(const std::string &problemText1, GLint errorCode); // Функция сообщения и архивирования критических ошибок
    GLint panic(const std::string &problemText1, GLfloat value); // Функция сообщения и архивирования критических ошибок

    GLint problem(const std::string &problemText1, const std::string &problemText2 = ""); // Функция сообщения и архивирования системных ошибок
    GLint problem(const std::string &problemText1, GLint errorCode); // Функция сообщения и архивирования системных ошибок
    GLint problem(const std::string &problemText1, GLfloat value); // Функция сообщения и архивирования системных ошибок

    GLfloat trimAngle(GLfloat deg);
}

inline GLfloat Vasnecov::trimAngle(GLfloat deg)
{
    if(deg > 0)
    {
        deg = fmod(deg, 360);
    }
    if(deg < 0)
    {
        deg = fmod(deg, 360) + 360;
    }
    return deg;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOV_TEHNOLOGIST_H
