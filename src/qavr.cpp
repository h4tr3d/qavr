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
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSplitter>

#include <iostream>

#include "qavr.h"
#include "util.h"
#include "bitscellwidget.h"
#include "version.h"

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
    qApp->setApplicationName("qavr");
    qApp->setApplicationVersion(APP_VERSION_FULL);
    std::cout << "Version: " << APP_VERSION_FULL << std::endl;

    _avrdude_process = new AvrdudeProcess(this);
    _avrdude_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(_avrdude_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this,             SLOT(avrdudeProcessFinished(int, QProcess::ExitStatus)));
    connect(_avrdude_process, SIGNAL(readyRead()),
            this,             SLOT(readyForReadAvrdude()));
    connect(_avrdude_process, SIGNAL(fusesAvail(QStringList)),
            this,             SLOT(fusesAvail(QStringList)));
    connect(_avrdude_process, SIGNAL(locksAvail()),
            this,             SLOT(locksAvail()));

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
    // Lock Editor
    //
    lock_table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    lock_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    //
    // Data and Settings
    //
    _data_dir_sys.setPath(DATA_PREFIX);
    _data_dir_user.setPath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    if (!_data_dir_user.exists())
        _data_dir_user.mkpath(_data_dir_user.path());

    QString tmp = QString("%1/%2-%3")
                    .arg(QDir::tempPath())
                    .arg(qApp->applicationName())
                    .arg(getUserName());
    _tmp_dir.setPath(tmp);
    if (!_tmp_dir.exists())
        _tmp_dir.mkpath(tmp);

    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "qavr", "qavr");
    QString   dir = QFileInfo(cfg.fileName()).absolutePath() + "/";
    _config_dir.setPath(dir);
    if (!_config_dir.exists())
    {
        _config_dir.mkpath(_config_dir.path());
    }
    _xdg_config = _config_dir.filePath("qavr.ini");

    //
    // Load MCUs
    //
    loadMCU(_data_dir_sys);
    loadMCU(_data_dir_user);

    QMap<QString, MCU>::iterator it;
    for (it = _mcu_list.begin(); it != _mcu_list.end(); it++)
    {
        mcu->addItem(it.key());
    }

    //
    // About info
    //
    loadAbout();

    //
    // Format list
    //
    loadFormats();

    //
    // Load settings
    //
    loadSettings();
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
        flash_file->setText(file);
    }
}

void QAvr::on_hex_upload_clicked()
{
    if (flash_file->text().isEmpty())
    {
        return;
    }

    prepareAvrdudeProcess();
    _avrdude_process->uploadFlash(flash_file->text(), _flash_format);
}

void QAvr::on_load_flash_clicked()
{
    prepareAvrdudeProcess();
    _avrdude_process->readFlash();
}

void QAvr::on_save_flash_clicked()
{
    QString save_file;

    save_file = QFileDialog::getSaveFileName(this, tr("Save HEX file"), "", "*.hex;;All files (*)");

    if (save_file.isEmpty())
    {
        return;
    }

    prepareAvrdudeProcess();
    _avrdude_process->saveFlash(save_file, _flash_format);
}

void QAvr::on_verify_flash_clicked()
{
    if (flash_file->text().isEmpty())
    {
        return;
    }

    prepareAvrdudeProcess();
    _avrdude_process->verifyFlash(flash_file->text(), _flash_format);
}


void QAvr::avrdudeProcessFinished(int, QProcess::ExitStatus)
{
    // TODO
    std::cout << "Finished\n";
}

// Read data from avrdude
void QAvr::readyForReadAvrdude()
{
    while(_avrdude_process->bytesAvailable() > 0)
    {
        QString data = flash_output->toPlainText();
        data += _avrdude_process->readLine();
        flash_output->setPlainText(data);
        flash_output->ensureCursorVisible(); // TODO: check
    }
}

void QAvr::fusesAvail(QStringList fuses)
{
    _fuses = _avrdude_process->getFuses();
    updateGuiFromFuses();

    Fuses f = _avrdude_process->getFuses();
    QString str;
    for (int i = 0; i < fuses.count(); i++)
    {
        str += QString("%1 = 0x%2, ").arg(fuses.at(i)).arg(f[fuses.at(i)], 0, 16);
    }
    std::cout << "Fuses avail: " << str.toAscii().data() << "\n";
}


// Load settings
void QAvr::loadSettings()
{
    QFile xdg_config_file(_xdg_config);
    QSettings settings(_xdg_config, QSettings::IniFormat);

    QString command = "avrdude";
    QString config  = "";
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
            config          = settings.value("avrdude_config").toString();
            type            = settings.value("programmer_type").toString();
            port            = settings.value("programmer_port").toString();
            speed           = settings.value("programmer_speed").toString();
            additional_args = settings.value("additional_args").toString();
        settings.endGroup();

        settings.beginGroup("Application");
            bool is_maximized = settings.value("is_maximized").toBool();
            if (is_maximized)
            {
                setWindowState(windowState() | Qt::WindowMaximized);
            }
            else
            {
                setGeometry(settings.value("geometry").toRect());
            }
        settings.endGroup();

        settings.beginGroup("Programmer");
            mcu->setEditText(settings.value("last_mcu").toString());
            flash_file->setText(settings.value("flash_file").toString());
            eeprom_file->setText(settings.value("eeprom_file").toString());
            flash_format->setCurrentIndex(settings.value("flash_format").toInt());
            eeprom_format->setCurrentIndex(settings.value("eeprom_format").toInt());

            flash_output_splitter->restoreState(settings.value("output_size").toByteArray());
            programmer_tabs->setCurrentIndex(settings.value("programmer_tab").toInt());
        settings.endGroup();

        settings.beginGroup("Fuses");
            fuse_names        = settings.value("names").toStringList();
            fuse_translations = settings.value("translations").toStringList();
        settings.endGroup();
    }

    avrdude_command->setText(command);
    config_file->setText(config);
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
        settings.setValue("avrdude_config",   config_file->text());
        settings.setValue("programmer_type",  programmer_type->currentText());
        settings.setValue("programmer_port",  programmer_port->text());
        settings.setValue("programmer_speed", programmer_speed->currentText());
        settings.setValue("additional_args",  avrdude_args->text());
    settings.endGroup();

    settings.beginGroup("Application");
        settings.remove("");
        settings.setValue("geometry",     geometry());
        settings.setValue("is_maximized", isMaximized());
    settings.endGroup();

    settings.beginGroup("Programmer");
        settings.remove("");
        settings.setValue("last_mcu",    mcu->currentText());

        settings.setValue("flash_file",  flash_file->text());
        settings.setValue("eeprom_file", eeprom_file->text());
        settings.setValue("flash_format", flash_format->currentIndex());
        settings.setValue("eeprom_format", eeprom_format->currentIndex());

        settings.setValue("output_size", flash_output_splitter->saveState());
        settings.setValue("programmer_tab", programmer_tabs->currentIndex());
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

void QAvr::readFuses()
{
    prepareAvrdudeProcess();
    _avrdude_process->readFuses(_unit.fuses);
}

void QAvr::writeFuses()
{
    prepareAvrdudeProcess();
    _avrdude_process->writeFuses(_unit.fuses, _fuses);
}

void QAvr::prepareAvrdudeProcess()
{
    QString mcu_name;

    stopAvrdudeProcess();
    flash_output->clear();

    _avrdude_process->setCommand(avrdude_command->text());
    _avrdude_process->setConfigFile(config_file->text());
    _avrdude_process->setProgrammerType(programmer_type->currentText());
    _avrdude_process->setProgrammerPort(programmer_port->text());
    _avrdude_process->setProgrammerSpeed(programmer_speed->currentText());
    _avrdude_process->setFuseTrans(_fuse_trans);
    _avrdude_process->setAdditionalArgs(q_parseCommandLineArgs(avrdude_args->text()));

    mcu_name = mcu->currentText();
    if (!_mcu_list[mcu_name].prog_name.isEmpty())
    {
        mcu_name = _mcu_list[mcu_name].prog_name;
    }
    _avrdude_process->setMcuName(mcu_name);
}

void QAvr::stopAvrdudeProcess()
{
    _avrdude_process->kill();
}


void QAvr::on_fuse_read_clicked()
{
    readFuses();
}


void QAvr::updateGuiFromFuses()
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

void QAvr::updateFusesFromGui()
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
    updateFusesFromGui();
}

// Load MCU list
void QAvr::loadMCU(QDir data_dir)
{
    QStringList filter;
    QFileInfoList mcu_list;
    QDir mcu_dir(data_dir.filePath("mcu"));

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

        unit.lock_names    = mcu_conf.value("lock_names").toStringList();
        unit.lock_comments = mcu_conf.value("lock_comments").toStringList();

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
    updateLockNamesAndComments();
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

    updateGuiFromFuses();
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

    updateFusesFromGui();
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

    about_data.replace("{{version}}", APP_VERSION_FULL) // TODO: fix package version
              .replace("{{image}}", ":/images/icon128.png");

    about_view->setHtml(about_data);
}

void QAvr::on_fuse_default_clicked()
{
    setFuseBitsToDefault(_unit);
}

// Close event processing
void QAvr::closeEvent(QCloseEvent *e)
{
    //ev->ignore(); // <--- application does not close
    //ev->accept(); // <--- application will be closed
    saveSettings();
    e->accept();
}


//
// EEPROM work
//

void QAvr::on_select_eeprom_clicked()
{
    QString file;

    file = QFileDialog::getOpenFileName(this, tr("Open EEPROM HEX file"),
                                        "", "*.ee.hex;;All files (*)");

    if (!file.isEmpty())
    {
        eeprom_file->setText(file);
    }

}

void QAvr::on_upload_eeprom_clicked()
{
    if (eeprom_file->text().isEmpty())
    {
        return;
    }

    prepareAvrdudeProcess();
    _avrdude_process->uploadEEPROM(eeprom_file->text(), _eeprom_format);
}

void QAvr::on_read_eeprom_clicked()
{
    prepareAvrdudeProcess();
    _avrdude_process->readEEPROM();
}

void QAvr::on_save_eeprom_clicked()
{
    QString save_file;

    save_file = QFileDialog::getSaveFileName(this, tr("Save EEPROM HEX file"),
                                             "", "*.ee.hex;;All files (*)");

    if (save_file.isEmpty())
    {
        return;
    }

    prepareAvrdudeProcess();
    _avrdude_process->saveEEPROM(save_file, _eeprom_format);
}

void QAvr::on_verify_eeprom_clicked()
{
    if (eeprom_file->text().isEmpty())
    {
        return;
    }

    prepareAvrdudeProcess();
    _avrdude_process->verifyEEPROM(eeprom_file->text(), _eeprom_format);
}


//
// Lock bits
//
void QAvr::on_lock_read_clicked()
{
    readLocks();
}

void QAvr::on_lock_write_clicked()
{
    writeLocks();
}

void QAvr::on_lock_default_clicked()
{
    setLockBitsToDefault(_unit);
}

void QAvr::updateLockTable(MCU unit)
{
    int bit;
    int column = 0;

    if (unit.lock_names.count() < 8)
    {
        lock_table->setColumnCount(0);
        return;
    }
    lock_table->setColumnCount(1);

    for (int row = 0; row < lock_table->rowCount() - 1; row++)
    {
        bit = 7 - row;
        BitsCellWidget *cell = new BitsCellWidget(0,
                                                  unit.lock_names.at(bit),
                                                  unit.lock_comments.at(bit));
        lock_table->setCellWidget(row, column, cell);

        connect(cell, SIGNAL(valueChanged(bool)),
                this,       SLOT(lockBitsToggled(bool)));
    }

    // Set up text widgets
    QLineEdit *line_edit = new QLineEdit();
    int line_number = lock_table->rowCount() - 1;
    lock_table->setCellWidget(line_number, column, line_edit);

    connect(line_edit, SIGNAL(textEdited(QString)),
            this,      SLOT(lockByteEdited(QString)));
}

void QAvr::lockBitsToggled(bool val)
{
    updateLocksFromGui();
}

void QAvr::lockByteEdited(QString text)
{
    int row    = lock_table->rowCount() - 1;
    int column = 0;

    QLineEdit *line_edit = static_cast<QLineEdit*>(lock_table->cellWidget(row, column));
    _lock = line_edit->text().toInt(0, 16);

    updateGuiFromLocks();
}

void QAvr::updateGuiFromLocks()
{
    unsigned char byte   = 0;
    int           column = 0;
    int           bit;

    byte = ~_lock;

    for (int row = 0; row < lock_table->rowCount() - 1; row++)
    {
        bit = 7 - row;
        BitsCellWidget *cell = static_cast<BitsCellWidget*>(lock_table->cellWidget(row, column));
        cell->setChecked(GetBit(byte, bit));
    }

    QLineEdit *edit = static_cast<QLineEdit*>(
                        lock_table->cellWidget(lock_table->rowCount() - 1, column));
    edit->setText(QString("0x%1").arg(_lock, 0, 16));
}

void QAvr::updateLocksFromGui()
{
    unsigned char byte   = 0;
    int           column = 0;

    int bit;

    byte = 0;
    for (int row = 0; row < lock_table->rowCount() - 1; row++)
    {
        bit = 7 - row;
        BitsCellWidget *cell = static_cast<BitsCellWidget*>(lock_table->cellWidget(row, column));

        if (cell->isChecked())
            SetBit(byte, bit);
    }

    byte = ~byte;
    _lock = byte;

    QLineEdit *edit = static_cast<QLineEdit*>(lock_table->cellWidget(lock_table->rowCount() - 1, column));
    edit->setText(QString("0x%1").arg(_lock, 0, 16));
}

void QAvr::updateLockNamesAndComments()
{
    QString mcu_name = mcu->currentText();
    _unit = _mcu_list[mcu_name];
    _lock = 0xFF;

    updateLockTable(_unit);
    setLockBitsToDefault(_unit);
}

void QAvr::setLockBitsToDefault(MCU unit)
{
    int bit;
    int column = 0;

    if (_unit.lock_names.count() < 8)
        return;

    for (int row = 0; row < lock_table->rowCount() - 1; row++)
    {
        bit = 7 - row;
        BitsCellWidget *cell = static_cast<BitsCellWidget*>(lock_table->cellWidget(row, column));

        cell->setChecked(false);
    }

    updateLocksFromGui();
}

void QAvr::locksAvail()
{
    _lock = _avrdude_process->getLocks();
    updateGuiFromLocks();

    QString str;
    str = QString("Lock = 0x%1, ").arg(_lock, 0, 16);
    std::cout << "Locks avail: " << str.toAscii().data() << std::endl;
}

void QAvr::readLocks()
{
    prepareAvrdudeProcess();
    _avrdude_process->readLocks();
}

void QAvr::writeLocks()
{
    prepareAvrdudeProcess();
    _avrdude_process->writeLocks(_lock);
}

// Set formats
void QAvr::loadFormats()
{
    QStringList format_names;
    _formats.clear();

    _formats     << AvrdudeProcess::FLASH_AUTO
                 << AvrdudeProcess::FLASH_INTEL_HEX
                 << AvrdudeProcess::FLASH_MOTOROLA
                 << AvrdudeProcess::FLASH_RAW;
    format_names << tr("Auto")
                 << tr("Intel Hex")
                 << tr("Motorola S-record")
                 << tr("RAW binary (little-endian)");

    flash_format->addItems(format_names);
    eeprom_format->addItems(format_names);
}

void QAvr::on_flash_format_currentIndexChanged(int index)
{
    _flash_format = _formats.at(index);
}

void QAvr::on_eeprom_format_currentIndexChanged(int index)
{
    _eeprom_format = _formats.at(index);
}
