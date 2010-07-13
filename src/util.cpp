/**************************************************************************
**   Copyright (C) 2010 by hatred
**   hatred@inbox.ru
**   http://hatred.homelinux.net
**
**   This file is a part of "%ProjectName%" application
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the version 2 of GNU General Public License as
**   published by the Free Software Foundation.
**
**   For more information see LICENSE and LICENSE.ru files
**
**   @file   %FileName%
**   @date   %DATE%
**   @author hatred
**   @brief
**
**************************************************************************/

#if defined(WIN32)
#   include <windows.h>
#   include <lmcons.h>
#else
#   include <pwd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

/**
 * Platform independ user name detection (currently win32 and *nix)
 * Don't pass return result to free(). Use strdup() for string duplication.
 *
 * @return NULL on error, null-terminated string with user name instead
 */
const char *getUserName()
{
    static char *user_name = NULL;
#if defined(WIN32)
    DWORD size = UNLEN + 1;

    if (user_name == NULL)
    {
        user_name = (char*)calloc(sizeof(char), size);
        if (user_name == NULL)
            return NULL;
    }

    GetUserName(user_name, &size);
#else
    struct passwd *passwd = getpwuid(getuid());
    if (passwd == NULL)
        return NULL;

    user_name = passwd->pw_name;
#endif

    return user_name;
}
