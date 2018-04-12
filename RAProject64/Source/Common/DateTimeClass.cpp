#include "stdafx.h"
#include "DateTimeClass.h"
#include <time.h>

CDateTime::CDateTime()
{
    m_time = time(NULL);
}

std::string CDateTime::Format(const char * format)
{
    char buffer[100];
    strftime(buffer, sizeof(buffer), format, localtime(&m_time));
    return std::string(buffer);
}

CDateTime & CDateTime::SetToNow(void)
{
    m_time = time(NULL);
    return *this;
}
