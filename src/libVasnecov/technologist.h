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
    int panic(const GLstring &problemText1, const GLstring &problemText2 = ""); // Функция сообщения и архивирования критических ошибок
    int panic(const GLstring &problemText1, int errorCode); // Функция сообщения и архивирования критических ошибок
    int panic(const GLstring &problemText1, float value); // Функция сообщения и архивирования критических ошибок

    int problem(const GLstring &problemText1, const GLstring &problemText2 = ""); // Функция сообщения и архивирования системных ошибок
    int problem(const GLstring &problemText1, int errorCode); // Функция сообщения и архивирования системных ошибок
    int problem(const GLstring &problemText1, float value); // Функция сообщения и архивирования системных ошибок

    float trimAngle(float deg);
}

inline float Vasnecov::trimAngle(float deg)
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
