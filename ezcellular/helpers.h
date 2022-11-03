/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include "enums.h"
#include "modem.h"
#include "structs.h"

#include <ostream>  // std::ostream

namespace ezcellular {

/* ostream helpers (e.g. std::cout) */

// enums
auto operator<<(std::ostream&, const Technology&) -> std::ostream&;
auto operator<<(std::ostream&, const IPType&) -> std::ostream&;

auto operator<<(std::ostream&, const Modem::ModemState&) -> std::ostream&;
auto operator<<(std::ostream&, const Modem::PowerState&) -> std::ostream&;
auto operator<<(std::ostream&, const Modem::LockState&) -> std::ostream&;

// structs
auto operator<<(std::ostream&, const SignalBase*) -> std::ostream& ;
auto operator<<(std::ostream&, const Signal&) -> std::ostream&;
auto operator<<(std::ostream&, const LocationBase*) -> std::ostream&;
auto operator<<(std::ostream&, const Location&) -> std::ostream&;
auto operator<<(std::ostream&, const CellInfoBase*) -> std::ostream&;
auto operator<<(std::ostream&, const CellInfo&) -> std::ostream&;

auto operator<<(std::ostream&, const IPConfig&) -> std::ostream&;
auto operator<<(std::ostream&, const TrafficStats&) -> std::ostream&;

} // namespace ezcellular
