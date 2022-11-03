/*
    SPDX-FileCopyrightText: 2023 Oliver Kästner <git@oliver-kaestner.de>
    SPDX-License-Identifier: LGPL-3.0-or-later
*/
#pragma once

#include <stdexcept>

namespace ezcellular {

class ModemManagerException : public std::runtime_error {
    using std::runtime_error::runtime_error;  // inherit ctor
};
class ModemException : public std::runtime_error {
    using std::runtime_error::runtime_error;  // inherit ctor
};
class SIMException : public std::runtime_error {
    using std::runtime_error::runtime_error;  // inherit ctor
};
class ConnectionException : public std::runtime_error {
    using std::runtime_error::runtime_error;  // inherit ctor
};

} // namespace ezcellular