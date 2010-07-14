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

AvrdudeProcess::AvrdudeProcess(QObject *parent) :
    QProcess(parent)
{
    _command          = "avrdude";
    _programmer_speed = "9600";

    _flash_format[FLASH_AUTO] = "a";
    _flash_format[FLASH_INTEL_HEX] = "i";
    _flash_format[FLASH_MOTOROLA] = "s";
    _flash_format[FLASH_BINARY] = "r";

    _is_fuses_avail = false;

    connect(this, SIGNAL(finished(int)),
            this, SLOT(processFinished(int)));

    _current_state = NONE_STATE;

    QString tmp = QString("%1/%2-%3")
                    .arg(QDir::tempPath())
                    .arg(qApp->applicationName()) // Application name should be setted before
                    .arg(getUserName());
    _tmp_dir.setPath(tmp);
    if (!_tmp_dir.exists())
        _tmp_dir.mkpath(tmp);

    setState(NONE_STATE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// State machine
////////////////////////////////////////////////////////////////////////////////////////////////////

void AvrdudeProcess::setState(States state)
{
    _current_state = state;
}

AvrdudeProcess::States AvrdudeProcess::getState()
{
    return _current_state;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Set/get commands
////////////////////////////////////////////////////////////////////////////////////////////////////

void AvrdudeProcess::setCommand(QString command)
{
    _command = command;
}

QString AvrdudeProcess::getCommand()
{
    return _command;
}

void AvrdudeProcess::setConfigFile(QString config_file)
{
    _config_file = config_file;
}

QString AvrdudeProcess::getConfigFile()
{
    return _config_file;
}

void AvrdudeProcess::setProgrammerType(QString programmer_type)
{
    _programmer_type = programmer_type;
}

QString AvrdudeProcess::getProgrammerType()
{
    return _programmer_type;
}

void AvrdudeProcess::setProgrammerSpeed(QString programmer_speed)
{
    _programmer_speed = programmer_speed;
}

QString AvrdudeProcess::getProgrammerSpeed()
{
    return _programmer_speed;
}

void AvrdudeProcess::setProgrammerPort(QString programmer_port)
{
    _programmer_port = programmer_port;
}

QString AvrdudeProcess::getProgrammerPort()
{
    return _programmer_port;
}

void AvrdudeProcess::setMcuName(QString mcu_name)
{
    _mcu_name = mcu_name;
}

QString AvrdudeProcess::getMcuName()
{
    return _mcu_name;
}

void AvrdudeProcess::setAdditionalArgs(QStringList additional_args)
{
    _additional_args = additional_args;
}

QStringList AvrdudeProcess::getAdditionalArgs()
{
    return _additional_args;
}

// Form common args list
QStringList AvrdudeProcess::formCommonArgsList()
{
    QStringList ret;

    if (!_config_file.isEmpty())
        ret << "-C" << _config_file;

    if (!_programmer_type.isEmpty())
        ret << "-c" << _programmer_type;

    if (!_programmer_speed.isEmpty())
        ret << "-B" << _programmer_speed;

    if (!_programmer_port.isEmpty())
        ret << "-P" << _programmer_port;

    if (!_mcu_name.isEmpty())
        ret << "-p" << _mcu_name;

    ret << _additional_args;

    return ret;
}

// avrdude process is finished, we analyse return values and current states and emit some signals
void AvrdudeProcess::processFinished(int exit_code)
{
    if (exit_code != 0)
    {
        emit avrdudeFail();
        setState(NONE_STATE);
        return;
    }

    if (getState() == READ_FUSE_STATE)
    {
        setState(DECODE_FUSE_STATE);
        unsigned char ch;

        for (int i = 0; i < _fuses.count(); i++)
        {
            QString avrdude_fuse_name = fuseName(_fuses.at(i));
            QString file_name = _tmp_dir.filePath("." + avrdude_fuse_name);
            ch = readFromFile(file_name);
            ch > 0 ? _values[_fuses.at(i)] = ch : _values[_fuses.at(i)] = 0xFF;
        }

        _is_fuses_avail = true;
        emit fusesAvail(_fuses);
    }

    setState(NONE_STATE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Work with fuse bits
////////////////////////////////////////////////////////////////////////////////////////////////////

// Read fuse bytes from unit
void AvrdudeProcess::readFuses(QStringList read_fuses)
{
    QStringList args = formCommonArgsList();
    setState(READ_FUSE_STATE);
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

// Write fuse bytes to unit
void AvrdudeProcess::writeFuses(QStringList write_fuses, Fuses fuses)
{
    QStringList args = formCommonArgsList();
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

// Check for fuse values avail
bool AvrdudeProcess::isFusesAvail()
{
    return _is_fuses_avail;
}

// Return current saved fuse values
Fuses AvrdudeProcess::getFuses()
{
    return _values;
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

// Set up fuse translation, like LOW -> lfuse, HIGH -> hfuse, EXTENDED -> efuse
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

// Convert internal flash format to avrdude format
QString AvrdudeProcess::flashFormat(FlashFormat format)
{
    QString format_str = _flash_format[format];

    if (format_str.isEmpty())
        format_str = "a";

    return format_str;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Common Flash/EEPROM files
////////////////////////////////////////////////////////////////////////////////////////////////////

void AvrdudeProcess::uploadCommon(QString memory, QString file, AvrdudeProcess::FlashFormat format)
{
    QStringList args = formCommonArgsList();
    QString format_str = flashFormat(format);

    args << "-e" << "-U" <<  memory + ":w:" + file + ":" + format_str;

    start(_command, args);

}

void AvrdudeProcess::readCommon(QString memory, AvrdudeProcess::FlashFormat format)
{
    QStringList args = formCommonArgsList();
    QString format_str = flashFormat(format);

    args << "-U" << memory + ":r:-:" + format_str;

    start(_command, args);
}

void AvrdudeProcess::saveCommon(QString memory, QString file, AvrdudeProcess::FlashFormat format)
{
    QStringList args = formCommonArgsList();
    QString format_str = flashFormat(format);

    args << "-U" << memory + ":r:" + file + ":" + format_str;

    start(_command, args);
}

void AvrdudeProcess::verifyCommon(QString memory, QString file, AvrdudeProcess::FlashFormat format)
{
    QStringList args = formCommonArgsList();
    QString format_str = flashFormat(format);

    args << "-U" << memory + ":v:" + file + ":" + format_str;

    start(_command, args);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Flash
////////////////////////////////////////////////////////////////////////////////////////////////////

// Upload flash file to unit
void AvrdudeProcess::uploadFlash(QString file, FlashFormat format)
{
    setState(UPLOAD_FLASH_STATE);
    uploadCommon("flash", file, format);
}

// Read flash, connect signal ReadyForRead for take flash code
void AvrdudeProcess::readFlash(AvrdudeProcess::FlashFormat format)
{
    setState(READ_FLASH_STATE);
    readCommon("flash", format);
}

// Save flash to given file
void AvrdudeProcess::saveFlash(QString file, AvrdudeProcess::FlashFormat format)
{
    setState(SAVE_FLASH_STATE);
    saveCommon("flash", file, format);
}

// Verify flash
void AvrdudeProcess::verifyFlash(QString file, AvrdudeProcess::FlashFormat format)
{
    setState(VERIFY_FLASH_STATE);
    verifyCommon("flash", file, format);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM
////////////////////////////////////////////////////////////////////////////////////////////////////

// Upload EEPROM file to unit
void AvrdudeProcess::uploadEEPROM(QString file, FlashFormat format)
{
    setState(UPLOAD_EEPROM_STATE);
    uploadCommon("eeprom", file, format);
}

// Read EEPROM, connect signal ReadyForRead for take flash code
void AvrdudeProcess::readEEPROM(AvrdudeProcess::FlashFormat format)
{
    setState(READ_EEPROM_STATE);
    readCommon("eeprom", format);
}

// Save EEPROM to given file
void AvrdudeProcess::saveEEPROM(QString file, AvrdudeProcess::FlashFormat format)
{
    setState(SAVE_EEPROM_STATE);
    saveCommon("eeprom", file, format);
}

// Verify EEPROM
void AvrdudeProcess::verifyEEPROM(QString file, AvrdudeProcess::FlashFormat format)
{
    setState(VERIFY_EEPROM_STATE);
    verifyCommon("eeprom", file, format);
}
