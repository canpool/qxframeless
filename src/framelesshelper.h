/**
 * Copyright (C) 2023 maminjie <canpool@163.com>
 * SPDX-License-Identifier: MulanPSL-2.0
**/
#ifndef FRAMELESSHELPER_H
#define FRAMELESSHELPER_H

#include "qxframeless_global.h"
#include <QObject>

QX_FRAMELESS_BEGIN_NAMESPACE

class FramelessHelperPrivate;

class QX_FRAMELESS_EXPORT FramelessHelper : public QObject
{
    Q_OBJECT
public:
    explicit FramelessHelper(QObject *parent = nullptr);

private:
    QX_DECLARE_PRIVATE(FramelessHelper)
};

QX_FRAMELESS_END_NAMESPACE

#endif // FRAMELESSHELPER_H
