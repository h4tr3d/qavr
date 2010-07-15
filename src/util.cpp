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
#include <ctype.h>

#include <iostream>

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

/**
 *  Function parse command line arguments string and convert it to list of strings
 *
 *  @param  program - command line string
 *  @return list of parsed arguments
 */
std::list<std::string> parseCommandLineArgs(const std::string &program)
{
    std::list<std::string> args;
    std::string            tmp;
    int                    quote_count = 0;
    bool                   in_quote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < program.size(); ++i)
    {
        if (program.at(i) == '"')
        {
            ++quote_count;
            if (quote_count == 3)
            {
                // third consecutive quote
                quote_count = 0;
                tmp += program.at(i);
            }
            continue;
        }

        if (quote_count)
        {
            if (quote_count == 1)
                in_quote = !in_quote;
            quote_count = 0;
        }

        if (!in_quote && isspace(program.at(i)))
        {
            if (!tmp.empty())
            {
                //args += tmp;
                args.push_back(tmp);
                tmp.clear();
            }
        }
        else
        {
            tmp += program.at(i);
        }
    }

    if (!tmp.empty())
        args.push_back(tmp);

    return args;
}

#ifdef HAVE_QT

/**
 * Qt4 version of parseCommandLineArgs()
 *
 * @param  args - command line arguments
 * @return List of strings
 */

QStringList q_parseCommandLineArgs(const QString &args)
{
    std::list<std::string> list;
    std::list<std::string>::iterator it;
    list = parseCommandLineArgs(args.toStdString());
    QStringList ret;

    for (it = list.begin(); it != list.end(); it++)
    {
        ret.push_back(QString::fromStdString(*it));
    }

    return ret;
}

#endif // HAVE_QT
