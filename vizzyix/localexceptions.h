#ifndef LOCALEXCEPTIONS_H
#define LOCALEXCEPTIONS_H

#include <stdexcept>

class NotDefinedException : public std::logic_error
{
  public:
    NotDefinedException() : std::logic_error("Function not defined")
    {
    }
};

#endif // LOCALEXCEPTIONS_H
