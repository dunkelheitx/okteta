/*
    This file is part of the Kasten Framework, made within the KDE community.

    Copyright 2009 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KASTEN_ABSTRACTLINEPOPUP_P_HPP
#define KASTEN_ABSTRACTLINEPOPUP_P_HPP

#include "abstractlinepopup.hpp"

class QIcon;
class QLabel;
class QHBoxLayout;
class QEventLoop;

namespace Kasten {

class AbstractLinePopupPrivate
{
public:
    explicit AbstractLinePopupPrivate(AbstractLinePopup* parent);
    AbstractLinePopupPrivate() = delete;

    ~AbstractLinePopupPrivate();

public:
    void init();

    void setPosition(QPoint globalPosition);
    void setIcon(const QIcon& icon);
    void setWidget(QWidget* widget);

    void setResult(int result);
    int exec();
    void open();
    void accept();
    void reject();

    int result() const;

public: // QWidget API
    void setVisible(bool visible);

public:
    bool isOrContainsObject(QObject* object) const;

private:
    void done(int result);

private:
    AbstractLinePopup* const p;

    QHBoxLayout* mBaseLayout;
    QLabel* mIconLabel;
    QWidget* mWidget = nullptr;

    QEventLoop* mEventLoop = nullptr;
    int mResult = AbstractLinePopup::Rejected;
};

inline AbstractLinePopupPrivate::AbstractLinePopupPrivate(AbstractLinePopup* parent)
    : p(parent)
{
}

inline AbstractLinePopupPrivate::~AbstractLinePopupPrivate() = default;

inline int AbstractLinePopupPrivate::result() const { return mResult; }
inline void AbstractLinePopupPrivate::setResult(int result) { mResult = result; }

inline bool AbstractLinePopupPrivate::isOrContainsObject(QObject* object) const
{
    bool result = false;

    while (object) {
        if (object == p) {
            result = true;
            break;
        }
        object = object->parent();
    }

    return result;
}

}

#endif
