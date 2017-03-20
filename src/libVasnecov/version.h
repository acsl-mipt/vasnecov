#ifndef VASNECOV_VERSION_H
#define VASNECOV_VERSION_H

#define VASNECOV_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

// Для программы sborkapp
#define VASNECOV_VERSION_MAJOR 1
#define VASNECOV_VERSION_MINOR 1
#define VASNECOV_VERSION_RELEASE 1
#define VASNECOV_VERSION_BUILD 102
#define VASNECOV_VERSION_REVISION 1
#define VASNECOV_VERSION_STATUS ""

#define VASNECOV_VERSION_KOD VASNECOV_VERSION(VASNECOV_VERSION_MAJOR,VASNECOV_VERSION_MINOR,VASNECOV_VERSION_RELEASE)

#include <QString>

namespace Vasnecov
{
    struct Version
    {
        int major, minor, release, build; // major, minor - какие-то функции-макросы, поэтому их не получится использоваться в init list
        int revision;
        QString status;
        QString versionText;

        Version()
        {
            major = VASNECOV_VERSION_MAJOR;
            minor = VASNECOV_VERSION_MINOR;
            release = VASNECOV_VERSION_RELEASE;
            build = VASNECOV_VERSION_BUILD;
            revision = VASNECOV_VERSION_REVISION;
            status = VASNECOV_VERSION_STATUS;

            versionText = (QString::number(VASNECOV_VERSION_MAJOR) + "." +
                           QString::number(VASNECOV_VERSION_MINOR) + "." +
                           QString::number(VASNECOV_VERSION_RELEASE) + "." +
                           QString::number(VASNECOV_VERSION_BUILD) +
                           VASNECOV_VERSION_STATUS);
        }
    };
}

#endif
