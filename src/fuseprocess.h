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
**   @file   fuseprocess.h
**   @date   2010-07-10
**   @author hatred
**   @brief  class for reading and writing fuse bits
**
**************************************************************************/

#ifndef FUSEPROCESS_H
#define FUSEPROCESS_H

#include <QDir>
#include <QMap>
#include <QProcess>
#include <QStringList>

typedef QMap<QString, unsigned char> Fuses;

class FuseProcess : public QProcess
{
    Q_OBJECT
public:
    explicit FuseProcess(QObject *parent = 0,
                         QString command = QString("avrdude"),
                         QStringList std_args = QStringList());

    void readFuses(QStringList read_fuses);
    void writeFuses(QStringList write_fuses, Fuses fuses = Fuses());

    bool isFusesAvail();
    Fuses getFuses();

    void setFuseTrans(QMap<QString, QString> trans);

private:
    enum States {
        NONE_STATE,
        WRITE_STATE,
        READ_STATE,
        DECODE_STATE
    };

    unsigned char readFromFile(QString file);
    QString       fuseName(QString name);

    QString     _command;
    QStringList _std_args;
    QStringList _fuses;
    Fuses       _values;
    QMap<QString, QString> _fuse_trans;

    QDir        _tmp_dir;

    bool          _is_fuses_avail;

    States        _current_state;

signals:
    void fusesAvail(QStringList fuses);

public slots:

private slots:
    void processFinished(int);
};

#endif // FUSEPROCESS_H
