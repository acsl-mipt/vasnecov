/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VASNECOV_H
#define VASNECOV_H

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#include "vasnecovscene.h"
#include "vasnecovuniverse.h"
#include "version.h"

#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

QString showVasnecovVersion();
Vasnecov::Version vasnecovVersion();

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
#endif // VASNECOV_H
