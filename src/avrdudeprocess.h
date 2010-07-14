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
    explicit AvrdudeProcess(QObject *parent = 0,
                            QString command = QString("avrdude"),
                            QStringList std_args = QStringList());

    // Fuses
    void readFuses(QStringList read_fuses);
    void writeFuses(QStringList write_fuses, Fuses fuses = Fuses());
    bool isFusesAvail();
    Fuses getFuses();
    void setFuseTrans(QMap<QString, QString> trans);

    // Flash

private:
    enum States {
        NONE_STATE,
        WRITE_FUSE_STATE,
        READ_FUSE_STATE,
        DECODE_FUSE_STATE
    };

    unsigned char readFromFile(QString file);
    QString       fuseName(QString name);

    QString     _command;
    QStringList _std_args;
    QDir        _tmp_dir;
    States      _current_state;

    QString     _config_file;
    QString     _programmer_type;
    QString     _programmer_speed;
    QString     _programmer_port;
    QString     _mcu_name;

    // Fuses vars
    QStringList            _fuses;
    Fuses                  _values;
    QMap<QString, QString> _fuse_trans;
    bool                   _is_fuses_avail;


signals:
    void fusesAvail(QStringList fuses);
    void avrdudeFail();

public slots:

private slots:
    void processFinished(int);
};

#endif // AVRDUDEPROCESS_H
