#include "technologist.h"
#include <iostream>
#include <sstream>
#include "bmcl/Logging.h"
#ifndef _MSC_VER
    #pragma GCC diagnostic warning "-Weffc++"
#endif

int Vasnecov::panic(const GLstring &problemText1, const GLstring &problemText2)
{
    BMCL_CRITICAL() << "3D: " << problemText1 << problemText2;
    return 1;
}


int Vasnecov::panic(const GLstring &problemText1, int errorCode)
{
    BMCL_CRITICAL() << "3D: "  << problemText1 << errorCode;
    return 1;
}


int Vasnecov::panic(const GLstring &problemText1, float value)
{
    BMCL_CRITICAL() << "3D: "  << problemText1 << value;
    return 1;
}


int Vasnecov::problem(const GLstring &problemText1, const GLstring &problemText2)
{
    BMCL_WARNING() << "3D: "  << problemText1 << problemText2;
    return 1;
}


int Vasnecov::problem(const GLstring &problemText1, int errorCode)
{
    BMCL_WARNING() << "3D: "  << problemText1 << errorCode;
    return 1;
}


int Vasnecov::problem(const GLstring &problemText1, float value)
{
    BMCL_WARNING() << "3D: "  << problemText1 << value;
    return 1;
}

#ifndef _MSC_VER
    #pragma GCC diagnostic ignored "-Weffc++"
#endif
