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

#ifndef UTIL_H
#define UTIL_H

#include <list>
#include <string>

// Detect Qt:
#if defined(QT_CORE_LIB) || defined(QT_GUI_LIB)
#define HAVE_QT
#endif

#ifdef HAVE_QT
#include <QString>
#include <QStringList>
#endif

//
// Macroses
//

// Set Bit in Byte
#define SetBit(byte, bit) (byte |= (1 << bit))

// Get Bit from Byte
#define GetBit(byte, bit) (byte & (1 << bit))

// Clear Bit in Byte
#define ClearBit(byte, bit) (byte &= ~(1 << bit))

//
// Definitions and enums
//


//
// Declarations
//
const char *getUserName();
std::list<std::string> parseCommandLineArgs(const std::string &program);

// Qt4 based decls
#ifdef HAVE_QT
QStringList q_parseCommandLineArgs(const QString &args);
#endif // HAVE_QT

#endif // UTIL_H
