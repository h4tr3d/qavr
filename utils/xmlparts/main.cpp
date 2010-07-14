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

#include <QtCore/QCoreApplication>

#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>

#include <iostream>

#include <math.h>

typedef struct {
    QString                    name;

    QStringList                fuses;
    QMap<QString, int>         fuses_bits_count;
    QMap<QString, QStringList> fuses_names;
    QMap<QString, QStringList> fuses_comments;
    QMap<QString, QStringList> fuses_defaults;

    QStringList                locks_names;
    QStringList                locks_comments;
} MCU;

QString name2avrdude(QString name)
{
    QString result;
    QString tmp = name.toLower();

    QStringList avrdude_mcu_list;

    // TODO: generate on fly
    avrdude_mcu_list
        << "t10" << "t8" << "t5" << "t4" << "ucr2" << "x128a4" << "x64a4" << "x32a4" << "x16a4"
        << "x256a3b" << "x256a3" << "x192a3" << "x128a3" << "x64a3" << "x256a1" << "x192a1"
        << "x128a1d" << "x128a1" << "x64a1" << "m6450" << "m3250" << "m645" << "m325" << "usb82"
        << "usb162" << "usb1287" << "usb1286" << "usb647" << "usb646" << "m32u4" << "t84" << "t44"
        << "t24" << "m128rfa1" << "m2561" << "m2560" << "m1281" << "m1280" << "m640" << "t85" << "t45"
        << "t25" << "pwm3b" << "pwm2b" << "pwm3" << "pwm2" << "t2313" << "m328p" << "t88" << "m168"
        << "m88" << "m48" << "t861" << "t461" << "t261" << "t26" << "m8535" << "m8515" << "m8"
        << "m161" << "m32" << "m6490" << "m649" << "m3290p" << "m3290" << "m329p" << "m329" << "m169"
        << "m163" << "m162" << "m1284p" << "m644p" << "m644" << "m324p" << "m164p" << "m16" << "c32"
        << "c64" << "c128" << "m128" << "m64" << "m103" << "8535" << "8515" << "4434" << "4433"
        << "2343" << "2333" << "2313" << "4414" << "1200" << "t15" << "t13" << "t12" << "t11";

    if (name.contains(QRegExp("^attiny", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 6);
        result = "t" + tmp;
    }
    else if (name.contains(QRegExp("^atmega", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 6);
        result = "m" + tmp;
    }
    else if (name.contains(QRegExp("^atxmega", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 7);
        result = "x" + tmp;
    }
    else if (name.contains(QRegExp("^at90s[0-9]", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 5);
        result = tmp;
    }
    else if (name.contains(QRegExp("^at90can[0-9]", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 7);
        result = "c" + tmp;
    }
    else if (name.contains(QRegExp("^at90usb[0-9]", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 7);
        result = "usb" + tmp;
    }
    else if (name.contains(QRegExp("^at90pwm[0-9]", Qt::CaseInsensitive)))
    {
        tmp.remove(0, 7);
        result = "pwm" + tmp;
    }
    //else if (name.contains(QRegExp("^at90pwm[0-9]", Qt::CaseInsensitive)))
    //{
    //
    //}

    if (!avrdude_mcu_list.contains(result))
    {
        std::cout << "Warning: can't found programmer unit for '"
                  << name.toAscii().data()
                  << "', should look like: '"
                  << result.toAscii().data()
                  << "'"
                  << std::endl;
        result = "";
    }

    return result;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString out_file = "mcu.ini";
    QString in_file;

    if (argc < 2)
    {
        std::cout << "Use: xmlpart part1 ... partn" << std::endl;
        std::cout << "  partn - is a Atmel XML descriptions (part of AvrStudio) like ATtiny2313.xml"
                  << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        QFile xml_file(argv[i]);
        xml_file.open(QIODevice::ReadOnly);
        QXmlStreamReader reader(&xml_file);

        QStringList stack;
        MCU mcu;
        int  current_bit = -1;
        bool is_xmega = false;
        while (!reader.atEnd())
        {
            reader.readNext();

            if (reader.isStartElement())
            {
                stack.push_back(reader.name().toString());
                QString stack_str = stack.join("/");
                QString last = stack.last();

                if (stack_str.contains(QRegExp("avrpart/fuse/.*/fuse[0-9]$", Qt::CaseInsensitive)))
                {
                    last.remove(0, 4);
                    current_bit = last.toInt();
                    std::cout << "Current bit: " << current_bit << std::endl;
                }
                else if (stack_str.contains(QRegExp("avrpart/fuse/fuse[0-9]*/text[0-9]$", Qt::CaseInsensitive)))
                {
                    last.remove(0, 4);
                    current_bit = 8 - last.toInt(); // TODO: value from from MASK
                    std::cout << "Current bit: " << current_bit << std::endl;
                }
                else if (stack_str.contains(QRegExp("avrpart/lockbit$", Qt::CaseInsensitive)))
                {
                    // init locks
                    std::cout << "LockByte begin" << std::endl;
                    for (int i = 0; i < 8; i++)
                    {
                        mcu.locks_names << "NA";
                        mcu.locks_comments << "";
                    }
                }
                else if (stack_str.contains(QRegExp("/lockbit/lockbit[0-9]*$", Qt::CaseInsensitive)))
                {
                    last.remove(0, 7);
                    current_bit = last.toInt();
                    std::cout << "Current bit: " << current_bit << std::endl;
                }
                else if (stack_str.contains(QRegExp("/lockbit/text[0-9]$", Qt::CaseInsensitive)) &&
                         is_xmega == true)
                {
                    last.remove(0, 4);
                    current_bit = 8 - last.toInt(); // TODO: value from from MASK
                    std::cout << "Current bit: " << current_bit << std::endl;
                }
            }
            else if (reader.isEndElement())
            {
                QString stack_str = stack.join("/");
                stack.removeLast();
                if (stack_str.contains(QRegExp("/fuse/.*/fuse[0-9]$", Qt::CaseInsensitive)) ||
                    stack_str.contains(QRegExp("/fuse/fuse[0-9]*/text[0-9]$", Qt::CaseInsensitive)) ||
                    stack_str.contains(QRegExp("/lockbit/lockbit[0-9]*$", Qt::CaseInsensitive)))
                {
                    current_bit = -1;
                }

            }
            else if (reader.isCharacters() && !reader.isWhitespace())
            {
                QString stack_str = stack.join("/");
                //std::cout << stack_str.toAscii().data() << std::endl;
                if (stack_str.contains("/admin/part_name", Qt::CaseInsensitive))
                {
                    mcu.name = reader.text().toString();
                    if (mcu.name.contains(QRegExp("^atxmega", Qt::CaseInsensitive)))
                    {
                        is_xmega = true;
                    }
                }
                else if (stack_str.contains("/fuse/list", Qt::CaseInsensitive))
                {
                    QString fuses = reader.text().toString();
                    fuses.remove(0, 1);
                    fuses.remove(fuses.length() - 1, 1);

                    mcu.fuses = fuses.split(":");
                    std::cout << "Fuses list: " << mcu.fuses.join(", ").toAscii().data() << std::endl;

                    // init
                    for (int i = 0; i < mcu.fuses.size(); i++)
                    {
                        QString current_fuse = mcu.fuses.at(i);
                        for (int j = 0; j < 8; j++)
                        {
                            mcu.fuses_names[current_fuse] << "NA";
                            mcu.fuses_comments[current_fuse] << "";
                            mcu.fuses_defaults[current_fuse] << "1";
                        }
                    }
                }
                else if (stack_str.contains("/fuse/", Qt::CaseInsensitive))
                {
                    // check fuses
                    for (int i = 0; i < mcu.fuses.size(); i++)
                    {
                        QString current_fuse = mcu.fuses.at(i);
                        QString check_line = "/fuse/" + current_fuse;
                        if (stack_str.contains(check_line, Qt::CaseInsensitive))
                        {
                            // inside fuse description
                            if (stack_str.contains(check_line + "/nmb_fuse_bits", Qt::CaseInsensitive))
                            {
                                mcu.fuses_bits_count[current_fuse] = reader.text().toString().toInt();
                                std::cout << "Number fuse bits: "
                                          << mcu.fuses_bits_count[current_fuse] << std::endl;

                            }
                            else if (stack_str.contains(QRegExp("/fuse[0-9]/name$", Qt::CaseInsensitive)))
                            {
                                mcu.fuses_names[current_fuse][current_bit] =  reader.text().toString();
                                std::cout << "Fuse " << current_fuse.toAscii().data()
                                          << "[" << current_bit << "].NAME = "
                                          << mcu.fuses_names[current_fuse][current_bit].toAscii().data()
                                          << std::endl;
                            }
                            else if (stack_str.contains(QRegExp("/fuse[0-9]/text$", Qt::CaseInsensitive)))
                            {
                                mcu.fuses_comments[current_fuse][current_bit] = reader.text().toString();
                                std::cout << "Fuse " << current_fuse.toAscii().data()
                                          << "[" << current_bit << "].TEXT = "
                                          << mcu.fuses_comments[current_fuse][current_bit].toAscii().data()
                                          << std::endl;
                            }
                            else if (stack_str.contains(QRegExp("/fuse[0-9]/default$", Qt::CaseInsensitive)))
                            {
                                mcu.fuses_defaults[current_fuse][current_bit] = reader.text().toString();
                                std::cout << "Fuse " << current_fuse.toAscii().data()
                                          << "[" << current_bit << "].DEFAULT = "
                                          << mcu.fuses_defaults[current_fuse][current_bit].toAscii().data()
                                          << std::endl;

                            }
                            // ATxMega
                            else if (stack_str.contains(QRegExp("/fuse[0-9]/text[0-9]/mask$", Qt::CaseInsensitive)))
                            {
                                // set current bit
                                int bit = reader.text().toString().toInt(0, 16);
                                if (log2(bit) >= 0 && log2(bit) <= 7)
                                {
                                    current_bit = log2(bit);
                                }
                                std::cout << "change Current bit to: " << current_bit << std::endl;
                            }
                            else if (stack_str.contains(QRegExp("/fuse[0-9]/text[0-9]/text$", Qt::CaseInsensitive)))
                            {
                                // set fuse name and comment
                                QString text = reader.text().toString();
                                QStringList splitted = text.split(" - ");
                                QString name     = QString("BIT%1").arg(current_bit);
                                QString comment  = text;

                                if (splitted.count() == 2)
                                {
                                    name    = splitted.at(1);
                                    comment = splitted.at(0);
                                }

                                mcu.fuses_names[current_fuse][current_bit]    = name;
                                mcu.fuses_comments[current_fuse][current_bit] = comment;
                                mcu.fuses_defaults[current_fuse][current_bit] = "1"; // Can't get info
                            }
                        }
                    }
                }
                else if (stack_str.contains("/lockbit/", Qt::CaseInsensitive))
                {
                    if (stack_str.contains(QRegExp("/lockbit[0-9]/name$", Qt::CaseInsensitive)))
                    {
                        mcu.locks_names[current_bit] =  reader.text().toString();
                        std::cout << "LockByte"
                                  << "[" << current_bit << "].NAME = "
                                  << mcu.locks_names[current_bit].toAscii().data()
                                  << std::endl;
                    }
                    else if (stack_str.contains(QRegExp("/lockbit[0-9]/text$", Qt::CaseInsensitive)))
                    {
                        mcu.locks_comments[current_bit] = reader.text().toString();
                        std::cout << "LockByte"
                                  << "[" << current_bit << "].TEXT = "
                                  << mcu.locks_comments[current_bit].toAscii().data()
                                  << std::endl;
                    }
                    else if (stack_str.contains(QRegExp("/text[0-9]/text$", Qt::CaseInsensitive)) &&
                             is_xmega == true)
                    {
                        // set fuse name and comment
                        QString text         = reader.text().toString();
                        QStringList splitted = text.split(" - ");
                        QString name         = QString("BIT%1").arg(current_bit);
                        QString comment      = text;

                        if (splitted.count() == 2)
                        {
                            name    = splitted.at(1);
                            comment = splitted.at(0);
                        }

                        mcu.locks_names[current_bit]    = name;
                        mcu.locks_comments[current_bit] = comment;
                    }

                }
            }

            if (reader.error())
            {
                std::cout << QCoreApplication::tr(
                            "Error: %1 in file %2 at line %3, column %4.\n").arg(
                                reader.errorString(), argv[i],
                                QString::number(reader.lineNumber()),
                                QString::number(reader.columnNumber())).toAscii().data();
            }
        }
        xml_file.close();

        // Write data
        QFileInfo fi(argv[i]);
        out_file = fi.completeBaseName().toLower() + ".ini";
        QSettings mcu_conf(out_file, QSettings::IniFormat);

        mcu_conf.beginGroup("MCU");
        mcu_conf.remove("");

        QString avrdude_unit = name2avrdude(mcu.name);

        //mcu_conf.setArrayIndex(i-1);
        mcu_conf.setValue("name",      mcu.name);
        mcu_conf.setValue("prog_name", avrdude_unit);
        mcu_conf.setValue("gcc_name",  "");
        mcu_conf.setValue("fuses", mcu.fuses);

        for (int i = 0; i < mcu.fuses.size(); i++)
        {
            QString fuse = mcu.fuses.at(i);
            QString fuse_name   = fuse + "_names";
            QString fuse_text   = fuse + "_comments";
            QString fuse_def    = fuse + "_defaults";

            mcu_conf.setValue(fuse_name, mcu.fuses_names[fuse]);
            mcu_conf.setValue(fuse_text, mcu.fuses_comments[fuse]);
            mcu_conf.setValue(fuse_def,  mcu.fuses_defaults[fuse]);
        }

        mcu_conf.setValue("lock_names",    mcu.locks_names);
        mcu_conf.setValue("lock_comments", mcu.locks_comments);

        mcu_conf.endGroup();
    }

    return 0;
}
