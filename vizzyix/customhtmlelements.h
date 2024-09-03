// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "HTML.h"

/*
 * Add a custom html element to HTML Builder
 */

namespace HTML
{

/*
 * Class to implement <hr> element
 */
class Hr : public Element
{
  public:
    Hr() : Element("hr")
    {
        mbVoid = true;
    }
};
} // namespace HTML

