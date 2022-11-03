/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/

/**
 * Include-all header. If installed in the system, just do:
 * #include <ezcellular/ezcellular.h>
 *
 */
#pragma once

// IWYU pragma: begin_exports
#include "connection.h"
#include "exception.h"
#include "enums.h"
#include "helpers.h"
#include "modem.h"
#include "modem_manager.h"
#include "sim.h"
#include "structs.h"
// IWYU pragma: end_exports
