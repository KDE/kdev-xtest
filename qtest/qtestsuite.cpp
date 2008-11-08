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

#include "qtestsuite.h"
#include "qtestcase.h"

using QTest::Suite;
using QTest::Case;
using Veritas::Test;

Suite::Suite(const QString& name, const QFileInfo& path, Test* parent)
    : Test(name, parent), m_path(path)
{
    setSelectionToggle(true);
}

Suite::~Suite()
{}

QFileInfo Suite::path()
{
    return m_path;
}

void Suite::setPath(const QFileInfo& path)
{
    m_path = path;
}

Case* Suite::child(int i) const
{
    return static_cast<Case*>(Test::child(i));
}

#include "qtestsuite.moc"
