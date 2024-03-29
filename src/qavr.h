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

#ifndef QAVR_H
#define QAVR_H

#include <QProcess>
#include <QStringList>
#include <QCloseEvent>

#include "ui_qavr.h"
#include "avrdudeprocess.h"

typedef struct MCU {

    MCU()
    {
        name = "";
        prog_name = "";
        gcc_name = "";
    }

    QString name;               /// Display name
    QString prog_name;          /// name for passing to programmer
    QString gcc_name;           /// TODO

    QStringList fuses;
    QMap<QString, QStringList> fuse_names;
    QMap<QString, QStringList> fuse_comments;
    QMap<QString, QStringList> fuse_defaults;

    QStringList                lock_names;
    QStringList                lock_comments;
} MCU;

class QAvr : public QWidget, private Ui::QAvr
{
    Q_OBJECT

public:
    explicit QAvr(QWidget *parent = 0);

    void     setMcu(const QString &new_mcu);
    void     setFlash(const QString &file);
    void     setEEPROM(const QString &file);
    void     setWorkTab(const QString &tab_name);

protected:
    void        changeEvent(QEvent     *e);
    void        closeEvent(QCloseEvent *e);

private:
    void        loadSettings();
    void        saveSettings();
    void        loadMCU(QDir data_dir);
    void        loadAbout();
    void        loadFormats();

    void        readFuses();
    void        writeFuses();

    void        readLocks();
    void        writeLocks();

    void        prepareAvrdudeProcess();
    void        stopAvrdudeProcess();

    void        updateGuiFromFuses();
    void        updateFusesFromGui();
    void        updateGuiFromLocks();
    void        updateLocksFromGui();
    void        updateFuseNamesAndComments();
    void        updateLockNamesAndComments();
    void        updateFuseTable(MCU unit);
    void        updateLockTable(MCU unit);

    void        setFuseBitsToDefault(MCU unit);
    void        setLockBitsToDefault(MCU unit);

    MCU         getUnitByName(const QString &mcu_name);

private:
    QMap<QString, MCU>     _mcu_list;
    QMap<QString, QString> _fuse_trans;
    QStringList            _programmer_types;

    QList<AvrdudeProcess::FlashFormat> _formats;
    AvrdudeProcess::FlashFormat        _flash_format;
    AvrdudeProcess::FlashFormat        _eeprom_format;

    AvrdudeProcess     *_avrdude_process;
    Fuses               _fuses;
    unsigned char       _lock;
    MCU                 _unit;

    QString             _xdg_config;
    QDir                _config_dir;
    QDir                _data_dir_sys;
    QDir                _data_dir_user;
    QDir                _tmp_dir;

private slots:
    void on_eeprom_format_currentIndexChanged(int index);
    void on_flash_format_currentIndexChanged(int index);
    void on_lock_default_clicked();
    void on_lock_write_clicked();
    void on_lock_read_clicked();
    void on_verify_eeprom_clicked();
    void on_save_eeprom_clicked();
    void on_read_eeprom_clicked();
    void on_upload_eeprom_clicked();
    void on_select_eeprom_clicked();
    void on_fuse_default_clicked();
    void on_mcu_currentIndexChanged(QString );
    void on_mcu_editTextChanged(QString );
    void on_fuse_write_clicked();
    void on_fuse_read_clicked();
    void on_save_settings_clicked();
    void on_verify_flash_clicked();
    void on_save_flash_clicked();
    void on_load_flash_clicked();
    void on_hex_upload_clicked();
    void on_select_hex_file_clicked();

private slots:
    void avrdudeProcessFinished(int, QProcess::ExitStatus);
    void readyForReadAvrdude();
    void fusesAvail(QStringList fuses);
    void locksAvail();

    void fuseBitsToggled(bool val);
    void fuseByteEdited(QString text);

    void lockBitsToggled(bool val);
    void lockByteEdited(QString text);

};

#endif // QAVR_H
