/*
    This file is part of the Kasten Framework, made within the KDE community.

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

#include "bytearraysourcecodestreamencoderconfigeditor.hpp"

// lib
#include "bytearraytextstreamencoderpreview.hpp"
// KF
#include <KLocalizedString>
#include <KComboBox>
// Qt
#include <QSpinBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>

namespace Kasten {

ByteArraySourceCodeStreamEncoderConfigEditor::ByteArraySourceCodeStreamEncoderConfigEditor(ByteArraySourceCodeStreamEncoder* encoder, QWidget* parent)
    : AbstractModelStreamEncoderConfigEditor(parent)
    , mEncoder(encoder)
{
    mSettings = mEncoder->settings();

    auto* pageLayout = new QFormLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);

    // variable name
    const QString variableNameLabel =
        i18nc("@label:textbox name of the created variable",
              "Name of variable:");

    mVariableNameEdit = new QLineEdit(this);
    mVariableNameEdit->setClearButtonEnabled(true);
    mVariableNameEdit->setText(mSettings.variableName);
    connect(mVariableNameEdit, &QLineEdit::textChanged, this, &ByteArraySourceCodeStreamEncoderConfigEditor::onSettingsChanged);
    pageLayout->addRow(variableNameLabel, mVariableNameEdit);

    // items per line
    const QString itemsPerLineLabel =
        i18nc("@label:textbox to define after how many items the list is wrapped",
              "Items per line:");

    mItemsPerLineEdit = new QSpinBox(this);
    mItemsPerLineEdit->setMinimum(1);
    mItemsPerLineEdit->setValue(mSettings.elementsPerLine);
    connect(mItemsPerLineEdit, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ByteArraySourceCodeStreamEncoderConfigEditor::onSettingsChanged);
    pageLayout->addRow(itemsPerLineLabel, mItemsPerLineEdit);

    // data type
    const QString dataTypeLabel =
        i18nc("@label:listbox the type of the data: char, integer, etc.",
              "Data type:");

    mDataTypeSelect = new KComboBox(this);
    const char* const* dataTypeNames = mEncoder->dataTypeNames();
    const int dataTypesCount = mEncoder->dataTypesCount();
    QStringList dataTypeNameStrings;
    dataTypeNameStrings.reserve(dataTypesCount);
    for (int i = 0; i < dataTypesCount; ++i) {
        dataTypeNameStrings << QLatin1String(dataTypeNames[i]);
    }

    mDataTypeSelect->addItems(dataTypeNameStrings);
    mDataTypeSelect->setCurrentIndex(static_cast<int>(mSettings.dataType));
    connect(mDataTypeSelect, QOverload<int>::of(&KComboBox::activated),
            this, &ByteArraySourceCodeStreamEncoderConfigEditor::onSettingsChanged);
    pageLayout->addRow(dataTypeLabel, mDataTypeSelect);

    // unsigned as hexadezimal
    const QString unsignedAsHexadecimalLabel =
        i18nc("@option:check Encode the values in hexadecimal instead of decimal, "
              "if the datatype has the property Unsigned",
              "Unsigned as hexadecimal:");

    mUnsignedAsHexadecimalCheck = new QCheckBox(this);
    mUnsignedAsHexadecimalCheck->setChecked(mSettings.unsignedAsHexadecimal);
    connect(mUnsignedAsHexadecimalCheck, &QCheckBox::toggled, this, &ByteArraySourceCodeStreamEncoderConfigEditor::onSettingsChanged);
    pageLayout->addRow(unsignedAsHexadecimalLabel, mUnsignedAsHexadecimalCheck);
}

ByteArraySourceCodeStreamEncoderConfigEditor::~ByteArraySourceCodeStreamEncoderConfigEditor() = default;

bool ByteArraySourceCodeStreamEncoderConfigEditor::isValid() const
{
    return true; // TODO: warn if not all selected bytes are used due to the data type length
}

AbstractSelectionView* ByteArraySourceCodeStreamEncoderConfigEditor::createPreviewView() const
{
    return new ByteArrayTextStreamEncoderPreview(mEncoder);
}

void ByteArraySourceCodeStreamEncoderConfigEditor::onSettingsChanged()
{
    mSettings.variableName = mVariableNameEdit->text();
    mSettings.elementsPerLine = mItemsPerLineEdit->value();
    mSettings.dataType = static_cast<SourceCodeStreamEncoderSettings::PrimitiveDataType>(mDataTypeSelect->currentIndex());
    mSettings.unsignedAsHexadecimal = mUnsignedAsHexadecimalCheck->isChecked();
    mEncoder->setSettings(mSettings);
}

}
