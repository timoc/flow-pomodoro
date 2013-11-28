/*
  This file is part of Flow.

  Copyright (C) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Sérgio Martins <sergio.martins@kdab.com>

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

#include "controller.h"
#include "taskmodel.h"
#include "settings.h"
#include "quickview.h"

#include <QTimer>
#include <QDebug>
#include <QGuiApplication>

enum {
    AfterAddingTimeout = 1000,
    TickInterval = 1000*60 // Ticks every minute
};

Controller::Controller(TaskModel *model, QObject *parent) : QObject(parent)
  , m_currentTaskDuration()
  , m_tickTimer(new QTimer(this))
  , m_afterAddingTimer(new QTimer(this))
  , m_elapsedMinutes(0)
  , m_expanded(false)
  , m_indexBeingEdited(-1)
  , m_model(model)
  , m_taskStatus(TaskStopped)
  , m_page(TheQueuePage)
  , m_selectedIndex(-1)
{
    m_tickTimer = new QTimer(this);
    m_tickTimer->setInterval(TickInterval);
    connect(m_tickTimer, SIGNAL(timeout()), SLOT(onTimerTick()));
    connect(m_afterAddingTimer, SIGNAL(timeout()), SIGNAL(firstSecondsAfterAddingChanged()));
    m_afterAddingTimer->setSingleShot(true);
    m_afterAddingTimer->setInterval(AfterAddingTimeout);

    m_defaultPomodoroDuration = Settings::instance()->value(QStringLiteral("defaultPomodoroDuration"), /*default=*/QVariant(25)).toInt();

    qApp->installEventFilter(this);
}

int Controller::remainingMinutes() const
{
    return m_currentTaskDuration - m_elapsedMinutes;
}

int Controller::currentTaskDuration() const
{
    return m_currentTaskDuration;
}

QString Controller::taskText() const
{
    return m_currentTask.text;
}

int Controller::indexBeingEdited() const
{
    return m_indexBeingEdited;
}

void Controller::setIndexBeingEdited(int index)
{
    if (m_indexBeingEdited != index) {
        m_indexBeingEdited = index;
    } else {
        m_indexBeingEdited = -1;
    }

    // qDebug() << "Controller::setIndexBeingEdited " << m_indexBeingEdited;
    emit indexBeingEditedChanged(m_indexBeingEdited);
}

void Controller::startPomodoro(int queueIndex)
{
    stopPomodoro(/*requeue=*/true); // Stop previous one, if any

    m_currentTask = m_model->at(queueIndex);
    m_model->removeTask(queueIndex);

    m_elapsedMinutes = 0;
    m_currentTaskDuration = m_defaultPomodoroDuration;

    setExpanded(false);

    m_tickTimer->start();
    m_afterAddingTimer->start();
    emit firstSecondsAfterAddingChanged();

    setTaskStatus(TaskStarted);
}

void Controller::stopPomodoro(bool reQueueTask)
{
    if (m_taskStatus == TaskStopped) {
        return;
    }

    if (reQueueTask && !m_currentTask.text.isEmpty()) {
        // Return it to the queue
        addTask(m_currentTask.text, false);
    }
    m_tickTimer->stop();
    m_elapsedMinutes = 0;
    m_currentTask = Task();

    setTaskStatus(TaskStopped);
}

void Controller::pausePomodoro()
{
    switch (m_taskStatus) {
    case TaskPaused:
        m_tickTimer->start();
        setTaskStatus(TaskStarted);
        break;
    case TaskStarted:
        m_tickTimer->stop();
        setTaskStatus(TaskPaused);
        break;
    default:
        break;
    }
}

void Controller::toggleSelectedIndex(int index)
{
    if (m_selectedIndex == index) {
        setSelectedIndex(-1);
    } else {
        setSelectedIndex(index);
    }
}

void Controller::cycleSelectionUp()
{
    if (m_selectedIndex == -1) {
        const int lastIndex = m_model->count()-1;
        setSelectedIndex(lastIndex);
    } else if (m_selectedIndex > 0){
        setSelectedIndex(m_selectedIndex-1);
    }
}

void Controller::cycleSelectionDown()
{
    const int lastIndex = m_model->count()-1;
    if (m_selectedIndex == -1 && lastIndex == -1) {
        return;
    }

    if (m_selectedIndex == -1) {
        setSelectedIndex(0);
    } else if (m_selectedIndex < lastIndex){
        setSelectedIndex(m_selectedIndex+1);
    }
}

bool Controller::expanded() const
{
    return m_expanded;
}

void Controller::setExpanded(bool expanded)
{
    if (expanded != m_expanded) {
        m_expanded = expanded;
        if (expanded) {
            QuickView::instance().requestActivate();
        } else {
            setIndexBeingEdited(-1);
        }
        setSelectedIndex(-1);
        emit expandedChanged(expanded);
    }
}

bool Controller::firstSecondsAfterAdding() const
{
    return m_afterAddingTimer->isActive();
}

Controller::Page Controller::currentPage() const
{
    return m_page;
}

void Controller::setCurrentPage(Controller::Page page)
{
    if (page != m_page) {
        m_page = page;
        emit currentPageChanged(page);
    }
}

TaskStatus Controller::taskStatus() const
{
    return m_taskStatus;
}

void Controller::setTaskStatus(TaskStatus status)
{
    if (status != m_taskStatus) {
        m_taskStatus = status;
        emit taskTextChanged();
        emit remainingMinutesChanged(remainingMinutes());
        emit taskStatusChanged();
        emit currentTaskDurationChanged(m_currentTaskDuration);
    }
}

void Controller::setDefaultPomodoroDuration(int duration)
{
    if (m_defaultPomodoroDuration != duration && duration > 0 && duration < 59) {
        m_defaultPomodoroDuration = duration;
        Settings::instance()->setValue("defaultPomodoroDuration", QVariant(duration));
        emit defaultPomodoroDurationChanged(duration);
    }
}

int Controller::defaultPomodoroDuration() const
{
    return m_defaultPomodoroDuration;
}

int Controller::selectedIndex() const
{
    return m_selectedIndex;
}

void Controller::setSelectedIndex(int index)
{
    if (index != m_selectedIndex) {
        m_selectedIndex = index;
        emit selectedIndexChanged(index);
    }
}

bool Controller::running() const
{
    return m_taskStatus == TaskStarted;
}

bool Controller::stopped() const
{
    return m_taskStatus == TaskStopped;
}

bool Controller::paused() const
{
    return m_taskStatus == TaskPaused;
}

void Controller::onTimerTick()
{
    //qDebug() << "Timer ticked";
    m_elapsedMinutes++;
    emit remainingMinutesChanged(remainingMinutes());

    if (remainingMinutes() == 0) {
        stopPomodoro(/**requeue=*/true);
        emit taskFinished();
    }
}


bool Controller::eventFilter(QObject *, QEvent *event)
{
    if (event->type() != QEvent::KeyRelease) {
        return false;
    }

    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

    const bool editing = m_indexBeingEdited != -1;

    if (editing && (keyEvent->key() != Qt::Key_Escape &&
                    keyEvent->key() != Qt::Key_Enter &&
                    keyEvent->key() != Qt::Key_Return)) {
        return false;
    }

    switch (keyEvent->key()) {
    case Qt::Key_Escape:
        if (editing) {
            setIndexBeingEdited(-1);
        } else {
            setExpanded(false);
        }
        return true;
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (editing) {
            const int index = m_indexBeingEdited;
            setIndexBeingEdited(-1);
            setSelectedIndex(index);
        } else {
            if (m_selectedIndex != -1) {
                startPomodoro(m_selectedIndex);
                setExpanded(false);
            } else {
                setExpanded(true);
            }
        }

        return true;
        break;
    case Qt::Key_Space:
        pausePomodoro();
        return true;
        break;
    case Qt::Key_S:
        stopPomodoro(true);
        return true;
        break;
    case Qt::Key_N:
        setExpanded(true);
        addTask("New Task", /**open editor=*/true);
        return true;
        break;
    case Qt::Key_Delete:
        if (m_selectedIndex == -1) {
            stopPomodoro(false);
        } else {
            removeTask(m_selectedIndex);
        }
        return true;
        break;
    case Qt::Key_Up:
        cycleSelectionUp();
        return true;
        break;
    case Qt::Key_Down:
        cycleSelectionDown();
        return true;
        break;
    case Qt::Key_E:
        if (m_selectedIndex != -1) {
            setIndexBeingEdited(m_selectedIndex);
            return true;
        }
        return false;
        break;
    default:
        break;
    }

    return false;
}

void Controller::addTask(const QString &text, bool startEditMode)
{
    m_model->addTask(text);
    setIndexBeingEdited(-1);

    if (startEditMode) {
        setExpanded(true);
        setIndexBeingEdited(m_model->rowCount()-1);
    }
}

void Controller::removeTask(int index)
{
    setIndexBeingEdited(-1);
    m_model->removeTask(index);
}

void Controller::updateTask(int index, const QString &newText)
{
    m_model->updateTask(index, newText);
}
