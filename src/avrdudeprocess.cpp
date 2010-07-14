/**************************************************************************
**   Copyright (C) 2010 by hatred
**   hatred@inbox.ru
**   http://hatred.homelinux.net
**
**   This file is a part of "QAvr" application
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the version 2 of GNU General Public License as
**   published by the Free Software Foundation.
**
**   For more information see LICENSE and LICENSE.ru files
**
**   @file   avrdudeprocess.cpp
**   @date   2010-07-10
**   @author hatred
**   @brief  class for reading and writing fuse bits
**
**************************************************************************/

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

#include "avrdudeprocess.h"
#include "util.h"

AvrdudeProcess::AvrdudeProcess(QObject *parent, QString command, QStringList std_args) :
    QProcess(parent)
{
    _command  = command;
    _std_args = std_args;

    _is_fuses_avail = false;

    connect(this, SIGNAL(finished(int)),
            this, SLOT(processFinished(int)));

    _current_state = NONE_STATE;

    QString tmp = QString("%1/%2-%3")
                    .arg(QDir::tempPath())
                    .arg(qApp->applicationName())
                    .arg(getUserName());
    _tmp_dir.setPath(tmp);
    if (!_tmp_dir.exists())
        _tmp_dir.mkpath(tmp);
}

void AvrdudeProcess::readFuses(QStringList read_fuses)
{
    QStringList args = _std_args;
    _current_state = READ_FUSE_STATE;
    _is_fuses_avail = false;

    // read fuses args
    for (int i = 0; i < read_fuses.count(); i++)
    {
        QString avrdude_fuse_name = fuseName(read_fuses.at(i));
        QString file_name = _tmp_dir.filePath("." + avrdude_fuse_name);
        QFile   file(file_name);

        if (file.exists())
        {
            file.remove();
        }

        _fuses = read_fuses;
        _values.clear();
        args << "-U" << QString("%1:r:%2:r").arg(avrdude_fuse_name).arg(file_name);
    }

    start(_command, args);
}

void AvrdudeProcess::writeFuses(QStringList write_fuses, Fuses fuses)
{
    QStringList args = _std_args;
    _current_state = WRITE_FUSE_STATE;

    // read fuses args
    for (int i = 0; i < write_fuses.count(); i++)
    {
        QString fuse = write_fuses.at(i);
        QString avrdude_fuse_name = fuseName(fuse);
        args << "-U" << QString("%1:w:0x%2:m").arg(avrdude_fuse_name).arg(fuses[fuse], 0, 16);
    }

    start(_command, args);
}

bool AvrdudeProcess::isFusesAvail()
{
    return _is_fuses_avail;
}

Fuses AvrdudeProcess::getFuses()
{
    return _values;
}

void AvrdudeProcess::processFinished(int exit_code)
{
    if (exit_code != 0)
    {
        emit avrdudeFail();
        _current_state = NONE_STATE;
        return;
    }

    if (_current_state == READ_FUSE_STATE)
    {
        _current_state = DECODE_FUSE_STATE;
        unsigned char ch;

        for (int i = 0; i < _fuses.count(); i++)
        {
            QString avrdude_fuse_name = fuseName(_fuses.at(i));
            QString file_name = _tmp_dir.filePath("." + avrdude_fuse_name);
            ch = readFromFile(file_name);
            ch > 0 ? _values[_fuses.at(i)] = ch : _values[_fuses.at(i)] = 0xFF;
        }

        _current_state = NONE_STATE;
        _is_fuses_avail = true;
        emit fusesAvail(_fuses);
    }
}

// Read fuse from file
unsigned char AvrdudeProcess::readFromFile(QString file_name)
{
    QFile     file(file_name);
    QFileInfo finfo( file );

    unsigned char fuse_byte = 0x00;

    if ( file.exists() && (finfo.size()==1) )
    {
        if (file.open(QIODevice::ReadOnly))
        {
            char ch;
            bool ret = file.getChar(&ch);

            if ( ret )
                fuse_byte = ch;

            file.close();
        }
    }

    return fuse_byte;
}

void AvrdudeProcess::setFuseTrans(QMap<QString, QString> trans)
{
    _fuse_trans = trans;
}

// Do fuse name translation into avrdude syntax
QString AvrdudeProcess::fuseName(QString name)
{
    QString new_name = _fuse_trans[name];
    if (new_name.isEmpty())
    {
        new_name = name.toLower();
    }

    return new_name;
}
