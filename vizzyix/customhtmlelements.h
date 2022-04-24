#ifndef CUSTOMHTMLELEMENTS_H
#define CUSTOMHTMLELEMENTS_H

#include "HTML.h"

/*
 * Add a custom html elements to HTML Builder
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

#endif // CUSTOMHTMLELEMENTS_H
