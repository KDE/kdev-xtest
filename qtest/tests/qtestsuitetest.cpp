/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "qtestsuitetest.h"

#include <kasserts.h>
#include <qtest_kde.h>
#include <qtestcase.h>

using QTest::Suite;
using QTest::Case;
using QTest::Test::SuiteTest;

namespace
{
QFileInfo someExe()
{
    return QFileInfo("some_exe.exe");
}
}

void SuiteTest::init()
{
    m_path = QFileInfo("/path/to/module");
    m_name = "s1";
    m_suite = new Suite(m_name, m_path, 0);
}

void SuiteTest::cleanup()
{
    delete m_suite;
}

void SuiteTest::construct()
{
    KOMPARE(m_suite->name(), m_name);
    KOMPARE(m_suite->path(), m_path);
}

void SuiteTest::addChildCase()
{
    KOMPARE(m_suite->childCount(), 0);

    Case* t = new Case("t1", someExe(), m_suite);
    m_suite->addChild(t);

    KOMPARE(m_suite->childCount(), 1);
    KOMPARE(m_suite->child(0), t);
}

void SuiteTest::addChildCases()
{
    Case* t1 = new Case("t1", someExe(), m_suite);
    Case* t2 = new Case("t2", someExe(), m_suite);
    m_suite->addChild(t1);
    m_suite->addChild(t2);

    KOMPARE(m_suite->childCount(), 2);
    KOMPARE(m_suite->child(0), t1);
    KOMPARE(m_suite->child(1), t2);
}

#include "qtestsuitetest.moc"
QTEST_KDEMAIN(SuiteTest, NoGUI)
