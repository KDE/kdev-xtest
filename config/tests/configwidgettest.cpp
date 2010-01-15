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

#include "configwidgettest.h"

#define protected public
#define private public
#include "../configwidget.h"
#undef private
#undef protected

#include <QtTest/QTest>
#include <QtTest/QTestKeyClicksEvent>
#include <qtest_kde.h>
#include "../../kasserts.h"
#include "fakedetailswidget.h"
#include <QComboBox>
#include <QLabel>
#include <KUrlRequester>
#include <qtoolbutton.h>
#include <KFileDialog>

using Veritas::ConfigWidgetTest;
using Veritas::ConfigWidget;

void ConfigWidgetTest::init()
{
    m_config = new ConfigWidget;
    m_config->setDetailsWidget(new FakeDetailsWidget());
    m_config->show();
}

void ConfigWidgetTest::cleanup()
{
    delete m_config;
}

// command
void ConfigWidgetTest::construct()
{
    // this allows you to play with the widget. adjust wait time.
    //QTest::qWait(5000);
}

// custom assertion
void ConfigWidgetTest::assertDetailsCollapsed()
{
    QList<FakeDetailsWidget*> fdw = m_config->findChildren<FakeDetailsWidget*>();
    Q_ASSERT(!fdw.isEmpty());
    KVERIFY(fdw[0]->isHidden());
}

// custom assertion
void ConfigWidgetTest::assertDetailsExpanded()
{
    QList<FakeDetailsWidget*> fdw = m_config->findChildren<FakeDetailsWidget*>();
    Q_ASSERT(!fdw.isEmpty());
    KVERIFY(fdw[0]->isVisible());
}

// command
void ConfigWidgetTest::detailsWidgetHiddenByDefault()
{
    // the details widget should not be visible on construction

    assertDetailsCollapsed();
}

// command
void ConfigWidgetTest::expandDetailsWidget()
{
    // click once on the expand detail button
    // the detail widget should be visible

    clickExpandDetails();
    assertDetailsExpanded();
}

// command
void ConfigWidgetTest::collapseDetailsWidget()
{
    // click twice on the expand detail button
    // now the details widget should be hidden again.

    clickExpandDetails();
    clickExpandDetails();
    assertDetailsCollapsed();
}

// custom assertion
void ConfigWidgetTest::assertTestExeFieldsShown(int numberOf)
{
    QList<KUrlRequester*> fdw = m_config->findChildren<KUrlRequester*>();
    KOMPARE(numberOf, fdw.count());
}

// command
void ConfigWidgetTest::noTestExeFieldShownByDefault()
{
    // by default no test-executable url bar should be shown

    assertTestExeFieldsShown(0);
}

// command
void ConfigWidgetTest::addTestExeField()
{
    // add a test-executable url-bar. the configwidget should now contain 1

    clickAddTestExeField();
    assertTestExeFieldsShown(1);
}

// command
void ConfigWidgetTest::addTwoTestExeFields()
{
    // add a second and third test-executable url-bar. the configwidget
    // should now contain 2 of them.

    clickAddTestExeField();
    clickAddTestExeField();
    assertTestExeFieldsShown(2);
    assertChildWidgetsEnabled();
}

// command
void ConfigWidgetTest::removeFirstTestExeField()
{
    // remove the single present (by default) test executable field
    // now the config widget should contain zero test-exe url bars.

    clickAddTestExeField();
    clickRemoveTestExeField(0); // the index of the field to remove
    assertTestExeFieldsShown(0);
}

// command
void ConfigWidgetTest::removeSecondTestExeField()
{
    // first add a second test exe bar, then remove it again.
    // now the config widget should contain a single test-exe url bar

    clickAddTestExeField();
    clickAddTestExeField();
    clickRemoveTestExeField(1); // the index of the field to remove
    assertTestExeFieldsShown(1);
}

// command
void ConfigWidgetTest::removeAllTestExeFieldsThenReAdd()
{
    // first add a second and third test exe bar. Then remove all 3 currently
    // present test exe bars. Finally add them again.
    // the configwidget should now contain 3 test exe bars.

    // add
    clickAddTestExeField();
    clickAddTestExeField();
    clickAddTestExeField();

    // remove
    clickRemoveTestExeField(0);
    clickRemoveTestExeField(0);
    clickRemoveTestExeField(0);

    // re-add
    clickAddTestExeField();
    clickAddTestExeField();
    clickAddTestExeField();

    assertTestExeFieldsShown(3);
}

void ConfigWidgetTest::noFrameworksComboBoxByDefault()
{
    // the test-framework selection box should be empty
    // by default

    QStringList frameworks = frameworkComboBoxContents();
    KVERIFY(frameworks.isEmpty());
    KOMPARE(-1, frameworkComboBox()->currentIndex());
}

void ConfigWidgetTest::singleFrameworkComboBox()
{
    // append a single framework to the box

    m_config->appendFramework("FooBar");
    QStringList frameworks = frameworkComboBoxContents();
    KOMPARE(1, frameworks.count());
    KOMPARE(QString("FooBar"), frameworks[0]);
    KOMPARE(-1, frameworkComboBox()->currentIndex());
}

void ConfigWidgetTest::multipleFrameworksComboBox()
{
    // append multiple frameworks to the selection box

    m_config->appendFramework("Foo");
    m_config->appendFramework("Bar");
    QStringList frameworks = frameworkComboBoxContents();
    KOMPARE(2, frameworks.count());
    KVERIFY(frameworks.contains("Foo"));
    KVERIFY(frameworks.contains("Bar"));
    KOMPARE(-1, frameworkComboBox()->currentIndex());
}

void ConfigWidgetTest::executableContents()
{
    // add test executable bars with actual urls filled
    // these urls should now be available through 'executables()'

    m_config->addTestExecutableField(KUrl("/foo"));
    m_config->addTestExecutableField(KUrl("http://bar"));

    KUrl::List executables = m_config->executables();
    KOMPARE(2, executables.count());
    KVERIFY(executables.contains(KUrl("/foo")));
    KVERIFY(executables.contains(KUrl("http://bar")));
}

// command
void ConfigWidgetTest::buttonsDisabledInReadOnlyMode()
{
    assertChildWidgetsEnabled();
    m_config->setReadOnly();
    assertChildWidgetsDisabled();
    m_config->addTestExecutableField(KUrl("/foo"));
    assertChildWidgetsDisabled();
}

/////////////// CUSTOM ASSERTIONS ////////////////////////////////////////////

void ConfigWidgetTest::assertChildWidgetsDisabled()
{
    QList<KUrlRequester*> exes = m_config->findChildren<KUrlRequester*>();
    foreach(KUrlRequester* exe, exes) {
       KVERIFY(!exe->isEnabled());
    }
    KVERIFY(!m_config->addExecutableButton()->isEnabled());
    foreach(QToolButton* b, m_config->m_removeButtons) {
       KVERIFY(!b->isEnabled());
    }
}

void ConfigWidgetTest::assertChildWidgetsEnabled()
{
    QList<QWidget*> children = m_config->findChildren<QWidget*>();
    foreach(QWidget* child, children) {
        KVERIFY_MSG(child->isEnabled(), child->metaObject()->className());
    }
}

////////////////////////// HELPERS ///////////////////////////////////////////

KUrlRequester* ConfigWidgetTest::fetchFirstTestExeRequester()
{
    QList<KUrlRequester*> reqs = m_config->findChildren<KUrlRequester*>();
    Q_ASSERT(reqs.size() == 1);
    KUrlRequester* req = reqs[0];
    return req;
}

void ConfigWidgetTest::clickExpandDetails() const
{
    m_config->expandDetailsButton()->toggle();
}

void ConfigWidgetTest::clickAddTestExeField() const
{
    m_config->addExecutableButton()->click();
}

void ConfigWidgetTest::clickRemoveTestExeField(int fieldIndex) const
{
    m_config->removeExecutableButton(fieldIndex)->click();
}

QComboBox* ConfigWidgetTest::frameworkComboBox() const
{
    return m_config->frameworkBox();
}

QStringList ConfigWidgetTest::frameworkComboBoxContents() const
{
    int nrof = m_config->frameworkBox()->count();
    QStringList frameworks;
    for (int i=0; i<nrof; ++i) {
        frameworks << m_config->frameworkBox()->itemText(i);
    }
    return frameworks;
}

QTEST_KDEMAIN( ConfigWidgetTest, GUI )
#include "configwidgettest.moc"

