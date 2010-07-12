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

#ifndef BITSCELLWIDGET_H
#define BITSCELLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>

class BitsCellWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BitsCellWidget(QWidget *parent = 0,
                            QString name = QString("-"),
                            QString text = QString(""));
    ~BitsCellWidget();

    void setName(QString name);
    void setComment(QString text);

    bool isChecked();
    void setChecked(bool checked);

private:
    QVBoxLayout *_layout;
    QLabel      *_text;
    QCheckBox   *_name;

signals:
    void valueChanged(bool value);

public slots:
    void checkBoxChanged(bool value);

};

#endif // BITSCELLWIDGET_H
