// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdexcept>

class NotDefinedException : public std::logic_error
{
  public:
    NotDefinedException() : std::logic_error("Function not defined")
    {
    }
};
