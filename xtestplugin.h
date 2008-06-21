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

#ifndef XTEST_XTESTPLUGIN_H
#define XTEST_XTESTPLUGIN_H

#include <interfaces/iplugin.h>
#include <QVariantList>

class KDevXtestPluginFactory;

/**
 * xUnit test runner plugin
 **/
class KDevXtestPlugin : public KDevelop::IPlugin
{
public:
    explicit KDevXtestPlugin(QObject* parent, const QVariantList & = QVariantList());
    virtual ~KDevXtestPlugin();
    //virtual Qt::DockWidgetArea dockWidgetAreaHint() const;

private:
    KDevXtestPluginFactory* m_xtestFactory;
};

#endif // XTEST_XTESTPLUGIN_H
