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

#ifndef VERSION_H
#define VERSION_H

/*
 * Consts:
 *  APP_VERSION      - main application version, can be redefined from command line
 *  APP_REVISION     - additional application version (like git build), can be redefined from command line
 *  APP_VERSION_FULL - version and revision info
 */

#ifndef  APP_VERSION
#   define APP_VERSION "0.2"
#endif

#ifdef GIT
#   include "gitinfo.h"
#   ifndef APP_REVISION
#       define APP_REVISION "git" GIT_REVISION
#   endif
#endif

#if defined(APP_REVISION)
#   define APP_VERSION_FULL APP_VERSION "-" APP_REVISION
#else
#   define APP_VERSION_FULL APP_VERSION
#endif

#endif // VERSION_H
