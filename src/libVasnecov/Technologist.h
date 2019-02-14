/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Types.h"

namespace Vasnecov
{
    GLint panic(const QString &problemText1, const QString &problemText2 = ""); // Функция сообщения и архивирования критических ошибок
    GLint panic(const QString &problemText1, GLint errorCode); // Функция сообщения и архивирования критических ошибок
    GLint panic(const QString &problemText1, GLfloat value); // Функция сообщения и архивирования критических ошибок

    GLint problem(const QString &problemText1, const QString &problemText2 = ""); // Функция сообщения и архивирования системных ошибок
    GLint problem(const QString &problemText1, GLint errorCode); // Функция сообщения и архивирования системных ошибок
    GLint problem(const QString &problemText1, GLfloat value); // Функция сообщения и архивирования системных ошибок

    GLfloat trimAngle(GLfloat deg);
}

inline GLfloat Vasnecov::trimAngle(GLfloat deg)
{
    if(deg > 0)
    {
        deg = std::fmod(deg, 360.0f);
    }
    if(deg < 0)
    {
        deg = std::fmod(deg, 360.0f) + 360.0f;
    }
    return deg;
}
