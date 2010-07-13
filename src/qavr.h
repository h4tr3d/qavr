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
#include "fuseprocess.h"

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
} MCU;

class QAvr : public QWidget, private Ui::QAvr
{
    Q_OBJECT

public:
    explicit QAvr(QWidget *parent = 0);

protected:
    void changeEvent(QEvent *e);

private:
    QStringList formCmdStdArgs();

    void        startFlashProcess(QStringList additional_args);
    void        stopFlashProcess();

    void        loadSettings();
    void        saveSettings();
    void        loadMCU();
    void        loadAbout();

    void        readFuses();
    void        writeFuses();
    void        stopFuseProcess();

    void        updateGuiFromData();
    void        updateDataFromGui();
    void        updateFuseNamesAndComments();
    void        updateFuseTable(MCU unit);

    void        setFuseBitsToDefault(MCU unit);

protected:
    void        closeEvent(QCloseEvent *ev);

private:
    QMap<QString, MCU>     _mcu_list;
    QMap<QString, QString> _fuse_trans;
    QStringList            _programmer_types;

    QProcess           *_flash_process;
    FuseProcess        *_fuse_process;
    Fuses               _fuses;
    MCU                 _unit;

    QString             _xdg_config;
    QDir                _config_dir;
    QDir                _data_dir_sys;
    QDir                _data_dir_user;
    QDir                _tmp_dir;

private slots:
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
    void readyForRead();
    void flashProcessFinished(int, QProcess::ExitStatus);

    void readyForReadFuses();
    void fusesAvail(QStringList fuses);

    void fuseBitsToggled(bool val);
    void fuseByteEdited(QString text);

};

#endif // QAVR_H
