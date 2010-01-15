/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 * Copyright 2010 Daniel Calviño Sánchez <danxuliu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QTEST_CONFIGWIDGETTEST_H_INCLUDED
#define QTEST_CONFIGWIDGETTEST_H_INCLUDED

#include <QtCore/QObject>
#include <KUrl>

class KUrlRequester;

namespace Veritas
{
class ConfigWidget;

/*! @unitundertest Veritas::ConfigWidget */
class ConfigWidgetTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void construct();
    void detailsWidgetHiddenByDefault();
    void expandDetailsWidget();
    void collapseDetailsWidget();

    void noTestExeFieldShownByDefault();
    void addTestExeField();
    void addTwoTestExeFields();
    void removeFirstTestExeField();
    void removeSecondTestExeField();
    void removeAllTestExeFieldsThenReAdd();

    void noFrameworksComboBoxByDefault();
    void singleFrameworkComboBox();
    void multipleFrameworksComboBox();

    void executableContents();
    void buttonsDisabledInReadOnlyMode();

private:
    void assertDetailsCollapsed();
    void assertDetailsExpanded();
    void assertTestExeFieldsShown(int numberOf);
    void assertChildWidgetsDisabled();
    void assertChildWidgetsEnabled();
    KUrlRequester* fetchFirstTestExeRequester();

    /*! Fakes a click on the expand details button. */
    void clickExpandDetails() const;

    /*! Fakes a click on the add test executable button. */
    void clickAddTestExeField() const;

    /*! Fakes a click on the remove test executable button.
     *  Since multiple test-exe bars are typically shown, @p fieldIndex says
     *  which one to remove. */
    void clickRemoveTestExeField(int fieldIndex) const;

    /*! Retrieves all the entries in the framework QComboBox */
    QStringList frameworkComboBoxContents() const;

private:
    ConfigWidget* m_config;
    KUrl m_expectedFolder; // input variable for assertFileDialogFolderEquals
    KUrlRequester* m_testExeInputField; // idem
};

}

#endif // QTEST_CONFIGWIDGETTEST_H_INCLUDED
