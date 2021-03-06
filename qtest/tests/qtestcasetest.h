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

#ifndef QXQTEST_QTESTCASETEST_H
#define QXQTEST_QTESTCASETEST_H

#include <QObject>
#include <QFileInfo>

namespace QTest {
class Case;
class Command;
class Executable;

/*! @unitundertest QTest::Case */
class CaseTest : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void construct();
    void addCommand();
    void addCommands();

    void clone_noChildren();
    void clone_singleChild();
    void clone_multipleChildren();
    void clone_properties();

private:
    void kompareCloneChild(Command* actualClone, Command* original, Case* parentClone);

private:
    QFileInfo m_exe;
    QString m_name;
    Case* m_case;
    Executable* m_executable;
};

}

#endif // QXQTEST_QTESTCASETEST_H
