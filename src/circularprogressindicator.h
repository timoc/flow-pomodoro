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

#ifndef CIRCULARPROGRESSINDICATOR_H
#define CIRCULARPROGRESSINDICATOR_H

#include <QColor>
#include <QQuickPaintedItem>

class CircularProgressIndicator : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int dpiFactor READ dpiFactor WRITE setDpiFactor NOTIFY dpiFactorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor WRITE setForegroundColor NOTIFY foregroundColorChanged)
    Q_PROPERTY(bool drawOutterBorder READ drawOutterBorder WRITE setDrawOutterBorder NOTIFY drawOutterBorderChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int minimumValue READ minimumValue WRITE setMinimumValue NOTIFY minimumValueChanged)
    Q_PROPERTY(int maximumValue READ maximumValue WRITE setMaximumValue NOTIFY maximumValueChanged)

public:
    explicit CircularProgressIndicator(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;

    int dpiFactor() const;
    void setDpiFactor(int);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor &);

    void setDrawOutterBorder(bool);
    bool drawOutterBorder() const;

    void setValue(int);
    int value() const;
    int maximumValue() const;
    void setMaximumValue(int);
    int minimumValue() const;
    void setMinimumValue(int);

Q_SIGNALS:
    void dpiFactorChanged();
    void foregroundColorChanged();
    void backgroundColorChanged();
    void drawOutterBorderChanged();
    void setProgressChanged();
    void valueChanged();
    void minimumValueChanged();
    void maximumValueChanged();

private:
    int m_dpiFactor;
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    bool m_drawOutterBorder;
    int m_value;
    int m_minimumValue;
    int m_maximumValue;
};

#endif
