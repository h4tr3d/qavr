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
**   @file   qavr.cpp
**   @date   2010-07-06
**   @author hatred
**   @brief
**
**************************************************************************/

#include <QFileInfoList>
#include <QFileDialog>
#include <QSettings>
#include <QBitArray>
#include <iostream>

#include "qavr.h"
#include "util.h"
#include "bitscellwidget.h"

//#undef DATA_PREFIX
#if !defined(DATA_PREFIX) || defined(WIN32)
#undef  DATA_PREFIX
#define DATA_PREFIX "."
#endif

QAvr::QAvr(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    setWindowIcon(QIcon(":/images/icon64.png"));

    _flash_process = NULL;
    _fuse_process  = NULL;

    // TODO: load from config? or generate?
    _programmer_types
                << "arduino"
                << "duemilanove"
                << "ft245r"
                << "ft232r"
                << "avrisp"
                << "avrispv2"
                << "avrispmkII"
                << "avrisp2"
                << "buspirate"
                << "stk500"
                << "stk500v1"
                << "mib510"
                << "stk500v2"
                << "stk500pp"
                << "stk500hvsp"
                << "stk600"
                << "stk600pp"
                << "stk600hvsp"
                << "avr910"
                << "usbasp"
                << "usbtiny"
                << "butterfly"
                << "avr109"
                << "avr911"
                << "jtagmkI"
                << "jtag1"
                << "jtag1slow"
                << "jtagmkII"
                << "jtag2slow"
                << "jtag2fast"
                << "jtag2"
                << "jtag2isp"
                << "jtag2dw"
                << "jtagmkII_avr32"
                << "jtag2avr32"
                << "jtag2pdi"
                << "dragon_jtag"
                << "dragon_isp"
                << "dragon_pp"
                << "dragon_hvsp"
                << "dragon_dw"
                << "dragon_pdi"
                << "pavr"
                << "bsd"
                << "stk200"
                << "pony-stk200"
                << "dt006"
                << "bascom"
                << "alf"
                << "sp12"
                << "picoweb"
                << "abcmini"
                << "futurlec"
                << "xil"
                << "dapa"
                << "atisp"
                << "ere-isp-avr"
                << "blaster"
                << "frank-stk200"
                << "89isp"
                << "ponyser"
                << "siprog"
                << "dasa"
                << "dasa3"
                << "c2n232i";

    programmer_type->addItems(_programmer_types);

    //
    // Fuse Editor
    //
    fuse_table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    fuse_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    //
    // MCUs
    //
    loadMCU();

    QMap<QString, MCU>::iterator it;
    for (it = _mcu_list.begin(); it != _mcu_list.end(); it++)
    {
        mcu->addItem(it.key());
    }

    //
    // Config
    //
    _xdg_config = getenv("XDG_CONFIG_HOME");
    if(_xdg_config.isEmpty())
    {
        _xdg_config  = getenv("HOME");
        _xdg_config += "/.config";
    }
    _xdg_config += "/qavr.ini";

    loadSettings();

    //
    // About info
    //
    loadAbout();

}

void QAvr::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

void QAvr::on_select_hex_file_clicked()
{
    QString file;

    file = QFileDialog::getOpenFileName(this, tr("Open HEX file"), "", "*.hex;;All files (*)");

    if (!file.isEmpty())
    {
        hex_file->setText(file);
    }
}

void QAvr::on_hex_upload_clicked()
{
    QStringList args;

    if (hex_file->text().isEmpty())
    {
        return;
    }

    args << "-e" << "-U" << "flash:w:" + hex_file->text();
    startFlashProcess(args);
}

void QAvr::on_load_flash_clicked()
{
    QStringList args;

    args << "-U" << "flash:r:-:i";
    startFlashProcess(args);
}

void QAvr::on_save_flash_clicked()
{
    QStringList args;
    QString save_file;

    save_file = QFileDialog::getSaveFileName(this, tr("Save HEX file"), "", "*.hex;;All files (*)");

    if (save_file.isEmpty())
    {
        return;
    }

    args << "-U" << "flash:r:" + save_file + ":i";
    startFlashProcess(args);
}

void QAvr::on_verify_flash_clicked()
{
    QStringList args;

    if (hex_file->text().isEmpty())
    {
        return;
    }

    args << "-U" << "flash:v:" + hex_file->text();
    startFlashProcess(args);
}

// Form command line for run
QStringList QAvr::formCmdStdArgs()
{
    QStringList ret;
    QString mcu_name;
    QString tmp_str;

    ret << "-c" << programmer_type->currentText()
        << "-B" << programmer_speed->currentText();

    tmp_str = programmer_port->text();
    if (!tmp_str.isEmpty())
        ret << "-P" << tmp_str;

    mcu_name = mcu->currentText();
    if (!_mcu_list[mcu_name].prog_name.isEmpty())
    {
        mcu_name = _mcu_list[mcu_name].prog_name;
    }

    ret << "-p" << mcu_name;

    return ret;
}

void QAvr::startFlashProcess(QStringList additional_args)
{
    if (_flash_process != NULL)
    {
        stopFlashProcess();
    }

    _flash_process = new QProcess(this);
    if (_flash_process == NULL)
    {
        return;
    }

    _flash_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(_flash_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this,           SLOT(flashProcessFinished(int,QProcess::ExitStatus)));
    connect(_flash_process, SIGNAL(readyRead()),
            this,           SLOT(readyForRead()));

    flash_output->clear();

    QStringList args = formCmdStdArgs();
    args << additional_args;
    QString command = avrdude_command->text();

    _flash_process->start(command, args);
}

void QAvr::stopFlashProcess()
{
    if (_flash_process != NULL)
        delete _flash_process;
}

void QAvr::readyForRead()
{
    while(_flash_process->bytesAvailable() > 0)
    {
        QString data = flash_output->toPlainText();
        data += _flash_process->readLine();
        flash_output->setPlainText(data);
    }
}


void QAvr::flashProcessFinished(int, QProcess::ExitStatus)
{
    // TODO
    std::cout << "Finished\n";
}

// Load settings
void QAvr::loadSettings()
{
    QFile xdg_config_file(_xdg_config);
    QSettings settings(_xdg_config, QSettings::IniFormat);

    QString command = "avrdude";
    QString type = "duemilanove";
    QString port = "002/002";
    QString speed = "19200";
    QString additional_args;

    QStringList fuse_names;
    QStringList fuse_translations;

    fuse_names        << "LOW"   << "HIGH"  << "EXTENDED" << "BYTE0";
    fuse_translations << "lfuse" << "hfuse" << "efuse"    << "fuse";

    if (xdg_config_file.exists())
    {
        settings.beginGroup("Settings");
            command         = settings.value("avrdude_command").toString();
            type            = settings.value("programmer_type").toString();
            port            = settings.value("programmer_port").toString();
            speed           = settings.value("programmer_speed").toString();
            additional_args = settings.value("additional_args").toString();
        settings.endGroup();

        settings.beginGroup("Fuses");
            fuse_names        = settings.value("names").toStringList();
            fuse_translations = settings.value("translations").toStringList();
        settings.endGroup();
    }

    avrdude_command->setText(command);
    programmer_type->setEditText(type);
    programmer_port->setText(port);
    programmer_speed->setEditText(speed);
    avrdude_args->setText(additional_args);

    for (int i = 0; i < fuse_names.count(); i++)
    {
        _fuse_trans[fuse_names.at(i)] = fuse_translations.at(i);
    }
}

// Save settings
void QAvr::saveSettings()
{
    QSettings settings(_xdg_config, QSettings::IniFormat);

    settings.beginGroup("Settings");
        settings.remove("");

        settings.setValue("avrdude_command",  avrdude_command->text());
        settings.setValue("programmer_type",  programmer_type->currentText());
        settings.setValue("programmer_port",  programmer_port->text());
        settings.setValue("programmer_speed", programmer_speed->currentText());
        settings.setValue("additional_args",  avrdude_args->text());
    settings.endGroup();

    settings.beginGroup("Fuses");
        settings.remove("");

        QStringList fuse_names;
        QStringList fuse_translations;

        QMap<QString, QString>::iterator it;
        for (it = _fuse_trans.begin(); it != _fuse_trans.end(); it++)
        {
            fuse_names << it.key();
            fuse_translations << it.value();
        }

        settings.setValue("names", fuse_names);
        settings.setValue("translations", fuse_translations);
    settings.endGroup();

}

void QAvr::on_save_settings_clicked()
{
    saveSettings();
}

void QAvr::readyForReadFuses()
{
    while(_fuse_process->bytesAvailable() > 0)
    {
        QString data = flash_output->toPlainText();
        data += _fuse_process->readLine();
        flash_output->setPlainText(data);
    }
}

void QAvr::readFuses()
{
    stopFuseProcess();

    _fuse_process = new FuseProcess(this, avrdude_command->text(), formCmdStdArgs());
    if (_fuse_process == NULL)
    {
        return;
    }

    _fuse_process->setProcessChannelMode(QProcess::MergedChannels);
    _fuse_process->setFuseTrans(_fuse_trans);

    connect(_fuse_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this,           SLOT(flashProcessFinished(int,QProcess::ExitStatus)));
    connect(_fuse_process, SIGNAL(readyRead()),
            this,           SLOT(readyForReadFuses()));
    connect(_fuse_process, SIGNAL(fusesAvail(QStringList)),
            this,          SLOT(fusesAvail(QStringList)));

    flash_output->clear();

    _fuse_process->readFuses(_unit.fuses);
}

void QAvr::writeFuses()
{
    stopFuseProcess();

    _fuse_process = new FuseProcess(this, avrdude_command->text(), formCmdStdArgs());
    if (_fuse_process == NULL)
    {
        return;
    }

    _fuse_process->setProcessChannelMode(QProcess::MergedChannels);
    _fuse_process->setFuseTrans(_fuse_trans);

    connect(_fuse_process, SIGNAL(finished(int,QProcess::ExitStatus)),
            this,           SLOT(flashProcessFinished(int,QProcess::ExitStatus)));
    connect(_fuse_process, SIGNAL(readyRead()),
            this,           SLOT(readyForReadFuses()));

    flash_output->clear();

    _fuse_process->writeFuses(_unit.fuses, _fuses);
}

void QAvr::stopFuseProcess()
{
    if (_fuse_process)
    {
        delete _fuse_process;
        _fuse_process = NULL;
    }
}

void QAvr::fusesAvail(QStringList fuses)
{
    _fuses = _fuse_process->getFuses();
    updateGuiFromData();

    Fuses f = _fuse_process->getFuses();
    QString str;
    for (int i = 0; i < fuses.count(); i++)
    {
        str += QString("%1 = 0x%2, ").arg(fuses.at(i)).arg(f[fuses.at(i)], 0, 16);
    }
    std::cout << "Fuses avail: " << str.toAscii().data() << "\n";
}

void QAvr::on_fuse_read_clicked()
{
    readFuses();
}


void QAvr::updateGuiFromData()
{
    unsigned char byte = 0;

    for (int i = 0; i < _unit.fuses.count(); i++)
    {
        QString fuse = _unit.fuses.at(i);
        int bit;

        byte = ~_fuses[fuse];

        for (int j = 0; j < fuse_table->rowCount() - 1; j++)
        {
            bit = 7 - j;
            BitsCellWidget *cell = static_cast<BitsCellWidget*>(fuse_table->cellWidget(j, i));
            cell->setChecked(GetBit(byte, bit));
        }

        QLineEdit *edit = static_cast<QLineEdit*>(fuse_table->cellWidget(fuse_table->rowCount() - 1, i));
        edit->setText(QString("0x%1").arg(_fuses[fuse], 0, 16));
    }
}

void QAvr::updateDataFromGui()
{
    unsigned char byte = 0;

    for (int i = 0; i < _unit.fuses.count(); i++)
    {
        QString fuse = _unit.fuses.at(i);
        int bit;

        byte = 0;
        for (int j = 0; j < fuse_table->rowCount() - 1; j++)
        {
            bit = 7 - j;
            BitsCellWidget *cell = static_cast<BitsCellWidget*>(fuse_table->cellWidget(j, i));

            if (cell->isChecked())
                SetBit(byte, bit);
        }

        byte = ~byte;
        _fuses[fuse] = byte;

        QLineEdit *edit = static_cast<QLineEdit*>(fuse_table->cellWidget(fuse_table->rowCount() - 1, i));
        edit->setText(QString("0x%1").arg(_fuses[fuse], 0, 16));
    }
}

void QAvr::on_fuse_write_clicked()
{
    writeFuses();
}

void QAvr::fuseBitsToggled(bool /*val*/)
{
    updateDataFromGui();
}

// Load MCU list
void QAvr::loadMCU()
{
    QStringList filter;
    QFileInfoList mcu_list;
    QDir mcu_dir(QString("%1/mcu").arg(DATA_PREFIX));

    filter << "*.ini";
    mcu_dir.setNameFilters(filter);

    mcu_list = mcu_dir.entryInfoList();
    for (int i = 0; i < mcu_list.size(); i++)
    {
        QFileInfo mcu_fi   = mcu_list.at(i);
        QString   mcu_file = mcu_dir.filePath(mcu_fi.fileName());
        QSettings mcu_conf(mcu_file, QSettings::IniFormat);
        MCU       unit;

        mcu_conf.beginGroup("MCU");

        unit.name           = mcu_conf.value("name").toString();
        unit.prog_name      = mcu_conf.value("prog_name").toString();
        unit.gcc_name       = mcu_conf.value("gcc_name").toString();

        unit.fuses          = mcu_conf.value("fuses").toStringList();
        for (int i = 0; i < unit.fuses.count(); i++)
        {
            QString     fuse = unit.fuses.at(i);
            unit.fuse_names[fuse]    = mcu_conf.value(fuse + "_names").toStringList();
            unit.fuse_comments[fuse] = mcu_conf.value(fuse + "_comments").toStringList();
            unit.fuse_defaults[fuse] = mcu_conf.value(fuse + "_defaults").toStringList();

            // fill empty items
            while (unit.fuse_names[fuse].count() <= 8)
            {
                unit.fuse_names[fuse] << "NA";
            }

            while (unit.fuse_comments[fuse].count() <= 8)
            {
                unit.fuse_comments[fuse] << "";
            }

            while (unit.fuse_names[fuse].count() <= 8)
            {
                unit.fuse_defaults[fuse] << "1";
            }
        }

        mcu_conf.endGroup();

        _mcu_list[unit.name] = unit;
    }
}

void QAvr::on_mcu_editTextChanged(QString text)
{
    on_mcu_currentIndexChanged(text);
}

void QAvr::on_mcu_currentIndexChanged(QString )
{
    updateFuseNamesAndComments();
}

void QAvr::updateFuseNamesAndComments()
{
    QString mcu_name = mcu->currentText();
    _unit = _mcu_list[mcu_name];
    _fuses.clear();

    updateFuseTable(_unit);
    setFuseBitsToDefault(_unit);
}

// Fuse entryes edited
void QAvr::fuseByteEdited(QString text)
{
    int        row       = fuse_table->rowCount() - 1;

    for (int i = 0; i < _unit.fuses.count(); i++)
    {
        QString    fuse      = _unit.fuses.at(i);
        QLineEdit *line_edit = static_cast<QLineEdit*>(fuse_table->cellWidget(row, i));

        _fuses[fuse] = line_edit->text().toInt(0, 16);
    }

    updateGuiFromData();
}

// resize table for MCUs fuses data
void QAvr::updateFuseTable(MCU unit)
{
    QStringList fuses = unit.fuses;

    // TODO: do or not disconnect for connected methods?
    fuse_table->setColumnCount(fuses.count());

    if (fuses.count() < 1)
        return;

    fuse_table->setHorizontalHeaderLabels(fuses);
    for (int column = 0; column < fuses.count(); column++)
    {
        QString fuse = fuses.at(column);

        int bit;
        for (int row = 0; row < fuse_table->rowCount() - 1; row++)
        {
            bit = 7 - row;
            BitsCellWidget *cell = new BitsCellWidget(0,
                                                      unit.fuse_names[fuse].at(bit),
                                                      unit.fuse_comments[fuse].at(bit));
            fuse_table->setCellWidget(row, column, cell);

            connect(cell, SIGNAL(valueChanged(bool)),
                    this,       SLOT(fuseBitsToggled(bool)));
        }

        // Set up text widgets
        QLineEdit *line_edit = new QLineEdit();
        int line_number = fuse_table->rowCount() - 1;
        fuse_table->setCellWidget(line_number, column, line_edit);

        connect(line_edit, SIGNAL(textEdited(QString)),
                this,      SLOT(fuseByteEdited(QString)));
    }
}

// set bits to default for given unit
void QAvr::setFuseBitsToDefault(MCU unit)
{
    for (int i = 0; i < unit.fuses.count(); i++)
    {
        QString fuse = unit.fuses.at(i);
        int bit;
        for (int j = 0; j < fuse_table->rowCount() - 1; j++)
        {
            bit = 7 - j;
            BitsCellWidget *cell = static_cast<BitsCellWidget*>(fuse_table->cellWidget(j, i));

            cell->setChecked(!unit.fuse_defaults[fuse].at(bit).toInt());
        }
    }

    updateDataFromGui();
}

// Setting up about display
void QAvr::loadAbout()
{
    QDir    data_dir(QString("%1").arg(DATA_PREFIX));
    QFile   about_file(data_dir.filePath("about.html"));
    QString about_data;

    if (!about_file.exists())
    {
        return;
    }

    if (!about_file.open(QIODevice::ReadOnly))
    {
        return;
    }
    about_data = about_file.readAll();
    about_file.close();

    about_data.replace("{{version}}", "0.1") // TODO: fix package version
              .replace("{{image}}", ":/images/icon128.png");

    about_view->setHtml(about_data);
}

void QAvr::on_fuse_default_clicked()
{
    setFuseBitsToDefault(_unit);
}
