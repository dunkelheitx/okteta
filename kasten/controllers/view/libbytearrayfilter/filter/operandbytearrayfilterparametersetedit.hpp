/*
    This file is part of the Okteta Kasten module, made within the KDE community.

    Copyright 2008 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KASTEN_OPERANDBYTEARRAYFILTERPARAMETERSETEDIT_HPP
#define KASTEN_OPERANDBYTEARRAYFILTERPARAMETERSETEDIT_HPP

// lib
#include "abstractbytearrayfilterparametersetedit.hpp"

namespace Okteta {
class ByteArrayComboBox;
}
class QCheckBox;

class OperandByteArrayFilterParameterSetEdit : public AbstractByteArrayFilterParameterSetEdit
{
    Q_OBJECT

public:
    static const char Id[];

public:
    explicit OperandByteArrayFilterParameterSetEdit(QWidget* parent = nullptr);
    ~OperandByteArrayFilterParameterSetEdit() override;

public: // AbstractByteArrayFilterParameterSetEdit API
    void setValues(const AbstractByteArrayFilterParameterSet* parameterSet) override;
    void setCharCodec(const QString& charCodecName) override;
    void getParameterSet(AbstractByteArrayFilterParameterSet* parameterSet) const override;
    bool isValid() const override;
    void rememberCurrentSettings() override;

protected Q_SLOTS:
    void onInputChanged(const QByteArray& data);

protected:
    Okteta::ByteArrayComboBox* mOperandEdit;
    QCheckBox* mAlignAtEndCheckBox;
};

#endif
