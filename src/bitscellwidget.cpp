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

#include <QFont>

#include "bitscellwidget.h"

BitsCellWidget::BitsCellWidget(QWidget *parent, QString name, QString text) :
    QWidget(parent)
{
    QFont font_label;
    QFont font_name;
    font_label.setItalic(true);
    font_name.setBold(true);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _layout = new QVBoxLayout(this);
    _name   = new QCheckBox();
    _text   = new QLabel();

    _name->setText(name);
    _name->setFont(font_name);

    _text->setText(text);
    _text->setWordWrap(true);
    _text->setFont(font_label);

    _layout->setSpacing(1);
    _layout->setContentsMargins(1, 1, 1, 1);
    _layout->addWidget(_name);
    _layout->addWidget(_text);

    connect(_name, SIGNAL(toggled(bool)),
            this,  SLOT(checkBoxChanged(bool)));
}

BitsCellWidget::~BitsCellWidget()
{
    // TODO
    _layout->removeWidget(_text);
    _layout->removeWidget(_name);
    delete _text;
    delete _name;
    delete _layout;
}

void BitsCellWidget::setName(QString name)
{
    _name->setText(name);
}

void BitsCellWidget::setComment(QString text)
{
    _text->setText(text);
}

void BitsCellWidget::checkBoxChanged(bool value)
{
    emit valueChanged(value);
}

bool BitsCellWidget::isChecked()
{
    return _name->isChecked();
}

void BitsCellWidget::setChecked(bool checked)
{
    _name->setChecked(checked);
}
