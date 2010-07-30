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

#include <QtGui/QApplication>
#include <QMessageBox>

#include <iostream>
#include <getopt.h>

#include "qavr.h"

using namespace std;

const char *options_string = "m:f:e:p:h";
const char *help_text =
"Use: qavr [-m <MCU name>] [-f <flash file>] [-e <EEPROM file>] [-p <process>] [-h]\n"
" Where <process> is:\n"
"   flash, eeprom, fuse, lock\n";

void use(const char *name)
{
#ifndef WIN32
    cout << help_text;
#else
    QMessageBox::information(0, QApplication::tr("QAvr usage information"), help_text);
#endif
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QAvr w;

    int opt = 0;
    while ((opt = getopt(argc, argv, options_string)) != -1)
    {
        switch (opt)
        {
            case 'm':
                w.setMcu(optarg);
                break;
            case 'f':
                w.setFlash(optarg);
                break;
            case 'e':
                w.setEEPROM(optarg);
                break;
            case 'p':
                w.setWorkTab(optarg);
                break;
            case 'h':
                use(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            default:
                use(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    w.show();
    return a.exec();
}
