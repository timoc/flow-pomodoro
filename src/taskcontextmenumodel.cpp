/*
  This file is part of Flow.

  Copyright (C) 2014 Sérgio Martins <iamsergio@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "taskcontextmenumodel.h"
#include "task.h"
#include "checkabletagmodel.h"

TaskContextMenuModel::TaskContextMenuModel(Task *task, QObject *parent)
    : QAbstractListModel(parent)
    , m_task(task)
{
    setObjectName("TaskContextMenuModel");

    Option option;
    option.text = tr("Edit ...");
    option.icon = "\uf044";
    m_staticOptions.insert(OptionTypeEdit, option);

    option.text = tr("Delete");
    option.icon = "\uf014";
    m_staticOptions.insert(OptionTypeDelete, option);

    option.text = tr("Configure Tags");
    option.icon = "\uf02b";
    m_staticOptions.insert(OptionTypeNewTag, option);

    m_moveToTodayOption.text = tr("Move to today's queue");
    m_moveToTodayOption.icon = "\uf06a";
    m_archiveOption.text = tr("Move to later queue");
    m_archiveOption.icon = "\uf187";

    connect(this, &TaskContextMenuModel::rowsInserted, this, &TaskContextMenuModel::countChanged);
    connect(this, &TaskContextMenuModel::rowsRemoved, this, &TaskContextMenuModel::countChanged);
    connect(this, &TaskContextMenuModel::modelReset, this, &TaskContextMenuModel::countChanged);
    connect(this, &TaskContextMenuModel::layoutChanged, this, &TaskContextMenuModel::countChanged);

    QAbstractItemModel *tagModel = m_task->checkableTagModel();
    connect(tagModel, &QAbstractItemModel::rowsInserted, this, &TaskContextMenuModel::onRowsInserted);
    connect(tagModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &TaskContextMenuModel::onRowsAboutToBeInserted);
    connect(tagModel, &QAbstractItemModel::rowsRemoved, this, &TaskContextMenuModel::onRowsRemoved);
    connect(tagModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &TaskContextMenuModel::onRowsAboutToBeRemoved);
    connect(tagModel, &QAbstractItemModel::modelReset, this, &TaskContextMenuModel::onModelReset);
    connect(tagModel, &QAbstractItemModel::modelAboutToBeReset, this, &TaskContextMenuModel::onModelAboutToBeReset);
    connect(tagModel, &QAbstractItemModel::layoutChanged, this, &TaskContextMenuModel::onLayoutChanged);
    connect(tagModel, &QAbstractItemModel::layoutAboutToBeChanged, this, &TaskContextMenuModel::onLayoutAboutToChange);
    connect(tagModel, &QAbstractItemModel::dataChanged, this, &TaskContextMenuModel::onDataChanged);
}

int TaskContextMenuModel::rowCount(const QModelIndex &/*parent*/) const
{
    return OptionTypeCount + m_task->checkableTagModel()->rowCount(QModelIndex());
}

QVariant TaskContextMenuModel::staticData(OptionType optionType, int role) const
{
    if (role == Qt::CheckStateRole)
        return false;

    if (role == TaskContextMenuModel::CheckableRole)
        return false;

    Option option;
    if (optionType == OptionTypeQueue) {
        option = m_task->staged() ? m_archiveOption : m_moveToTodayOption;
    } else {
        option = m_staticOptions.value(optionType);
    }

    if (role == TextRole)
        return option.text;
    else if (role == IconRole)
        return option.icon;

    return QVariant();
}

QVariant TaskContextMenuModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount(QModelIndex()))
        return QVariant();

    if (index.row() < OptionTypeCount)
        return staticData(static_cast<OptionType>(index.row()), role);

    int tagRow = index.row() - OptionTypeCount;
    QAbstractItemModel *tagModel = m_task->checkableTagModel();
    if (tagRow < 0 || tagRow >= tagModel->rowCount()) {
        qWarning() << "TaskContextMenuModel: invalid index" << tagRow;
        return QVariant();
    }

    QModelIndex tagIndex = tagModel->index(tagRow, 0);

    switch (role) {
    case TextRole:
        return tagIndex.data(CheckableTagModel::ItemTextRole);
    case IconRole:
        return "";
    case CheckableRole:
        return true;
    case Qt::CheckStateRole:
        return tagIndex.data(Qt::CheckStateRole);
    }

    return QVariant();
}

QHash<int, QByteArray> TaskContextMenuModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(TextRole, "textRole");
    roles.insert(IconRole, "iconRole");
    roles.insert(CheckableRole, "checkableRole");
    roles.insert(Qt::CheckStateRole, "checkedRole");

    return roles;
}

int TaskContextMenuModel::count() const
{
    return rowCount(QModelIndex());
}

void TaskContextMenuModel::onModelAboutToBeReset()
{
    beginResetModel();
}

void TaskContextMenuModel::onModelReset()
{
    endResetModel();
}

void TaskContextMenuModel::onLayoutAboutToChange()
{
    emit layoutAboutToBeChanged();
}

void TaskContextMenuModel::onLayoutChanged()
{
    emit layoutChanged();
}

void TaskContextMenuModel::onDataChanged(const QModelIndex &left, const QModelIndex &right)
{
    emit dataChanged(index(left.row() + OptionTypeCount, 0), index(right.row() + OptionTypeCount, 0));
}

void TaskContextMenuModel::onRowsAboutToBeInserted(const QModelIndex &, int start, int end)
{
    beginInsertRows(QModelIndex(), start + OptionTypeCount, end + OptionTypeCount);
}

void TaskContextMenuModel::onRowsInserted()
{
    endInsertRows();
}

void TaskContextMenuModel::onRowsAboutToBeRemoved(const QModelIndex &, int start, int end)
{
    beginRemoveRows(QModelIndex(), start + OptionTypeCount, end + OptionTypeCount);
}

void TaskContextMenuModel::onRowsRemoved()
{
    endRemoveRows();
}