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
**   @file   avrdudeprocess.h
**   @date   2010-07-10
**   @author hatred
**   @brief  class for work with avrdude
**
**************************************************************************/

#ifndef AVRDUDEPROCESS_H
#define AVRDUDEPROCESS_H

#include <QDir>
#include <QMap>
#include <QProcess>
#include <QStringList>

typedef QMap<QString, unsigned char> Fuses;

class AvrdudeProcess : public QProcess
{
    Q_OBJECT
public:
    enum FlashFormat {
        FLASH_AUTO,
        FLASH_INTEL_HEX,
        FLASH_MOTOROLA,
        FLASH_RAW
    };

public:
    explicit AvrdudeProcess(QObject *parent = 0);

    // Common
    void    setCommand(QString command);
    QString getCommand();
    void    setConfigFile(QString config_file);
    QString getConfigFile();
    void    setProgrammerType(QString programmer_type);
    QString getProgrammerType();
    void    setProgrammerSpeed(QString programmer_speed);
    QString getProgrammerSpeed();
    void    setProgrammerPort(QString programmer_port);
    QString getProgrammerPort();
    void    setMcuName(QString mcu_name);
    QString getMcuName();
    void        setAdditionalArgs(QStringList additional_args);
    QStringList getAdditionalArgs();

    QStringList formCommonArgsList();

    // Fuses
    void readFuses(QStringList read_fuses);
    void writeFuses(QStringList write_fuses, Fuses fuses = Fuses());
    bool isFusesAvail();
    Fuses getFuses();
    void setFuseTrans(QMap<QString, QString> trans);

    // Locks
    void          readLocks();
    void          writeLocks(unsigned char lock = 0xFF);
    bool          isLockssAvail();
    unsigned char getLocks();

    // Flash
    void uploadFlash(QString file, AvrdudeProcess::FlashFormat format = FLASH_AUTO);
    void readFlash(AvrdudeProcess::FlashFormat format = FLASH_INTEL_HEX);
    void saveFlash(QString file, AvrdudeProcess::FlashFormat format = FLASH_INTEL_HEX);
    void verifyFlash(QString file, AvrdudeProcess::FlashFormat format = FLASH_INTEL_HEX);

    // EEPROM
    void uploadEEPROM(QString file, AvrdudeProcess::FlashFormat format = FLASH_AUTO);
    void readEEPROM(AvrdudeProcess::FlashFormat format = FLASH_INTEL_HEX);
    void saveEEPROM(QString file, AvrdudeProcess::FlashFormat format = FLASH_INTEL_HEX);
    void verifyEEPROM(QString file, AvrdudeProcess::FlashFormat format = FLASH_INTEL_HEX);

private:
    // Common types
    enum States {
        NONE_STATE,

        // Fuse states
        WRITE_FUSE_STATE,
        READ_FUSE_STATE,
        DECODE_FUSE_STATE,

        // Lock states
        WRITE_LOCK_STATE,
        READ_LOCK_STATE,
        DECODE_LOCK_STATE,

        // Flash states
        UPLOAD_FLASH_STATE,
        READ_FLASH_STATE,
        SAVE_FLASH_STATE,
        VERIFY_FLASH_STATE,

        // EEPROM states
        UPLOAD_EEPROM_STATE,
        READ_EEPROM_STATE,
        SAVE_EEPROM_STATE,
        VERIFY_EEPROM_STATE
    };

    // Common functions
    void   setState(States state);
    States getState();

    // Fuse functions
    unsigned char readFromFile(QString file);
    QString       fuseName(QString name);

    // Flash/EEPROM common
    void uploadCommon(QString memory, QString file, AvrdudeProcess::FlashFormat format);
    void readCommon(QString memory, AvrdudeProcess::FlashFormat format);
    void saveCommon(QString memory, QString file, AvrdudeProcess::FlashFormat format);
    void verifyCommon(QString memory, QString file, AvrdudeProcess::FlashFormat format);

    // Flash functions
    QString       flashFormat(FlashFormat format);

    // Common vars
    QDir        _tmp_dir;
    States      _current_state;

    QString     _command;
    QString     _config_file;
    QString     _programmer_type;
    QString     _programmer_speed;
    QString     _programmer_port;
    QString     _mcu_name;
    QStringList _additional_args;

    // Fuses vars
    QStringList            _fuses;
    Fuses                  _fuse_values;
    QMap<QString, QString> _fuse_trans;
    bool                   _is_fuses_avail;

    // Locks vars
    unsigned char          _lock_value;
    bool                   _is_locks_avail;

    // Flash, EEPROM vars
    QMap<FlashFormat, QString> _flash_format;

signals:
    void fusesAvail(QStringList fuses);
    void locksAvail();
    void avrdudeFail();

public slots:

private slots:
    void processFinished(int);
};

#endif // AVRDUDEPROCESS_H
