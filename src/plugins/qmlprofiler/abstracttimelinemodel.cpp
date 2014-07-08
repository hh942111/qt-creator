/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "abstracttimelinemodel.h"
#include "abstracttimelinemodel_p.h"

namespace QmlProfiler {

AbstractTimelineModel::AbstractTimelineModel(AbstractTimelineModelPrivate *dd,
        const QString &name, const QString &label, QmlDebug::Message message,
        QmlDebug::RangeType rangeType, QObject *parent) :
    QObject(parent), d_ptr(dd)
{
    Q_D(AbstractTimelineModel);
    d->q_ptr = this;
    d->name = name;
    d->modelId = 0;
    d->modelManager = 0;
    d->expanded = false;
    d->title = label;
    d->message = message;
    d->rangeType = rangeType;
}

AbstractTimelineModel::~AbstractTimelineModel()
{
    Q_D(AbstractTimelineModel);
    delete d;
}

void AbstractTimelineModel::setModelManager(QmlProfilerModelManager *modelManager)
{
    Q_D(AbstractTimelineModel);
    d->modelManager = modelManager;
    connect(d->modelManager->qmlModel(),SIGNAL(changed()),this,SLOT(dataChanged()));
    d->modelId = d->modelManager->registerModelProxy();
}

QString AbstractTimelineModel::name() const
{
    Q_D(const AbstractTimelineModel);
    return d->name;
}

int AbstractTimelineModel::count() const
{
    Q_D(const AbstractTimelineModel);
    return d->count();
}

int AbstractTimelineModel::findFirstIndex(qint64 startTime) const
{
    Q_D(const AbstractTimelineModel);
    return d->findFirstIndex(startTime);
}

int AbstractTimelineModel::findFirstIndexNoParents(qint64 startTime) const
{
    Q_D(const AbstractTimelineModel);
    return d->findFirstIndexNoParents(startTime);
}

int AbstractTimelineModel::findLastIndex(qint64 endTime) const
{
    Q_D(const AbstractTimelineModel);
    return d->findLastIndex(endTime);
}

qint64 AbstractTimelineModel::getDuration(int index) const
{
    Q_D(const AbstractTimelineModel);
    return d->duration(index);
}

qint64 AbstractTimelineModel::getStartTime(int index) const
{
    Q_D(const AbstractTimelineModel);
    return d->startTime(index);
}

qint64 AbstractTimelineModel::getEndTime(int index) const
{
    Q_D(const AbstractTimelineModel);
    return d->startTime(index) + d->duration(index);
}

bool AbstractTimelineModel::isEmpty() const
{
    return count() == 0;
}

int AbstractTimelineModel::rowHeight(int rowNumber) const
{
    Q_D(const AbstractTimelineModel);
    if (!expanded())
        return DefaultRowHeight;

    if (d->rowOffsets.size() > rowNumber)
        return d->rowOffsets[rowNumber] - (rowNumber > 0 ? d->rowOffsets[rowNumber - 1] : 0);
    return DefaultRowHeight;
}

int AbstractTimelineModel::rowOffset(int rowNumber) const
{
    Q_D(const AbstractTimelineModel);
    if (rowNumber == 0)
        return 0;
    if (!expanded())
        return DefaultRowHeight * rowNumber;

    if (d->rowOffsets.size() >= rowNumber)
        return d->rowOffsets[rowNumber - 1];
    if (!d->rowOffsets.empty())
        return d->rowOffsets.last() + (rowNumber - d->rowOffsets.size()) * DefaultRowHeight;
    return rowNumber * DefaultRowHeight;
}

void AbstractTimelineModel::setRowHeight(int rowNumber, int height)
{
    Q_D(AbstractTimelineModel);
    if (!expanded())
        return;
    if (height < DefaultRowHeight)
        height = DefaultRowHeight;

    int nextOffset = d->rowOffsets.empty() ? 0 : d->rowOffsets.last();
    while (d->rowOffsets.size() <= rowNumber)
        d->rowOffsets << (nextOffset += DefaultRowHeight);
    int difference = height - d->rowOffsets[rowNumber] +
            (rowNumber > 0 ? d->rowOffsets[rowNumber - 1] : 0);
    if (difference != 0) {
        for (; rowNumber < d->rowOffsets.size(); ++rowNumber) {
            d->rowOffsets[rowNumber] += difference;
        }
        emit rowHeightChanged();
    }
}

int AbstractTimelineModel::height() const
{
    Q_D(const AbstractTimelineModel);
    int depth = rowCount();
    if (!expanded() || d->rowOffsets.empty())
        return depth * DefaultRowHeight;

    return d->rowOffsets.last() + (depth - d->rowOffsets.size()) * DefaultRowHeight;
}

qint64 AbstractTimelineModel::traceStartTime() const
{
    Q_D(const AbstractTimelineModel);
    return d->modelManager->traceTime()->startTime();
}

qint64 AbstractTimelineModel::traceEndTime() const
{
    Q_D(const AbstractTimelineModel);
    return d->modelManager->traceTime()->endTime();
}

qint64 AbstractTimelineModel::traceDuration() const
{
    Q_D(const AbstractTimelineModel);
    return d->modelManager->traceTime()->duration();
}

const QVariantMap AbstractTimelineModel::getEventLocation(int index) const
{
    Q_UNUSED(index);
    QVariantMap map;
    return map;
}

int AbstractTimelineModel::getEventIdForTypeIndex(int typeIndex) const
{
    Q_UNUSED(typeIndex);
    return -1;
}

int AbstractTimelineModel::getEventIdForLocation(const QString &filename, int line, int column) const
{
    Q_UNUSED(filename);
    Q_UNUSED(line);
    Q_UNUSED(column);
    return -1;
}

int AbstractTimelineModel::getBindingLoopDest(int index) const
{
    Q_UNUSED(index);
    return -1;
}

float AbstractTimelineModel::getHeight(int index) const
{
    Q_UNUSED(index);
    return 1.0f;
}

int AbstractTimelineModel::rowMinValue(int rowNumber) const
{
    Q_UNUSED(rowNumber);
    return 0;
}

int AbstractTimelineModel::rowMaxValue(int rowNumber) const
{
    Q_UNUSED(rowNumber);
    return 0;
}

void AbstractTimelineModel::dataChanged()
{
    Q_D(AbstractTimelineModel);
    switch (d->modelManager->state()) {
    case QmlProfilerDataState::ProcessingData:
        loadData();
        break;
    case QmlProfilerDataState::ClearingData:
        clear();
        break;
    default:
        break;
    }

    d->rowOffsets.clear();
}

bool AbstractTimelineModel::eventAccepted(const QmlProfilerDataModel::QmlEventTypeData &event) const
{
    Q_D(const AbstractTimelineModel);
    return (event.rangeType == d->rangeType && event.message == d->message);
}

bool AbstractTimelineModel::expanded() const
{
    Q_D(const AbstractTimelineModel);
    return d->expanded;
}

void AbstractTimelineModel::setExpanded(bool expanded)
{
    Q_D(AbstractTimelineModel);
    if (expanded != d->expanded) {
        d->expanded = expanded;
        emit expandedChanged();
    }
}

const QString AbstractTimelineModel::title() const
{
    Q_D(const AbstractTimelineModel);
    return d->title;
}

}
