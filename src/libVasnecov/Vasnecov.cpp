/*
 * Copyright (C) 2017 ACSL MIPT.
 * See the COPYRIGHT file at the top-level directory.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Vasnecov.h"

static Vasnecov::Version vVersion;
const QString& showVasnecovVersion()
{
    return vVersion.versionText;
}
Vasnecov::Version vasnecovVersion()
{
    return vVersion;
}
