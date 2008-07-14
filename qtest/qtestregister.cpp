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

#include "qtestregister.h"
#include "qtestsuite.h"
#include "qtestcase.h"
#include "qtestcommand.h"

#include <QIODevice>
#include <KLocalizedString>
#include <KDebug>

using QTest::QTestRegister;
using QTest::QTestSuite;
using QTest::QTestCase;
using QTest::QTestCommand;
using Veritas::Test;

const QString QTestRegister::c_suite("suite");
const QString QTestRegister::c_case("case");
const QString QTestRegister::c_cmd("command");
const QString QTestRegister::c_root("root");
const QString QTestRegister::c_name("name");
const QString QTestRegister::c_dir("dir");
const QString QTestRegister::c_exe("exe");

QTestRegister::QTestRegister()
        : m_root(""), m_settings(0)
{
        // Data for column headers is stored in the root item.
    QList<QVariant> rootData;
    rootData << i18n("Test Name") << i18n("Result") << i18n("Message")
             << i18n("File Name") << i18n("Line Number");
    m_rootItem = new Test(rootData);
}

QTestRegister::~QTestRegister()
{
}

Veritas::Test* QTestRegister::rootItem()
{
    return m_rootItem;
}

bool QTestRegister::isStartElement_(const QString& elem)
{
    return isStartElement() && (name() == elem);
}

bool QTestRegister::isEndElement_(const QString& elem)
{
    return isEndElement() && (name() == elem);
}

void QTestRegister::addFromXml(QIODevice* dev)
{
    Q_ASSERT(dev != 0);
    setDevice(dev);
    if (!device()->isOpen()) 
        device()->open(QIODevice::ReadOnly);

    while (!atEnd())
    {
        readNext();
        if (isStartElement_(c_root) && m_root.isEmpty())
            m_root = attributes().value(c_dir).toString();
        if (isStartElement_(c_suite))
            processSuite();
    }

    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

void QTestRegister::setRootDir(const QString& root)
{
    m_root = root;
}

void QTestRegister::processSuite()
{
    QTestSuite* suite = new QTestSuite(fetchName(), fetchDir(), m_rootItem);
    m_rootItem->addChild(suite);
    kDebug() << suite->name();

    while (!atEnd() && !isEndElement_(c_suite))
    {
        readNext();
        if (isStartElement_(c_case))
            processCase(suite);
    }
}

void QTestRegister::processCase(QTestSuite* suite)
{
    QTestCase* caze = new QTestCase(fetchName(), fetchExe(), suite);
    suite->addChild(caze);
    caze->setSettings(m_settings);
    kDebug() << caze->name();
    while (!atEnd() && !isEndElement_(c_case))
    {
        readNext();
        if (isStartElement_(c_cmd))
            processCmd(caze);
    }
}

void QTestRegister::processCmd(QTestCase* caze)
{
    QTestCommand* cmd = new QTestCommand(fetchName(), caze);
    caze->addChild(cmd);
    kDebug() << cmd->name();
    kDebug() << cmd->command();
}

QString QTestRegister::fetchName()
{
    return attributes().value(c_name).toString();
}

QFileInfo QTestRegister::fetchDir()
{
    QString dir = attributes().value(c_dir).toString();
    if (!m_root.isEmpty())
        dir = m_root + dir;
    return QFileInfo(dir);
}

QFileInfo QTestRegister::fetchExe()
{
    return QFileInfo(attributes().value(c_exe).toString());
}
