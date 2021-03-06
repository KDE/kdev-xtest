/*
 * This file is part of KDevelop
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

#include "qtestsettings.h"
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <KSharedConfig>
#include <KConfigGroup>


using QTest::Settings;
using QTest::ISettings;
using KDevelop::IProject;
using KDevelop::IBuildSystemManager;

ISettings::ISettings()
{}

ISettings::~ISettings()
{}

Settings::Settings(IProject* p)
    : m_project(p)
{}

Settings::~Settings()
{}

bool Settings::printAsserts() const
{
    KSharedConfig::Ptr cfg = m_project->projectConfiguration();
    KConfigGroup group(cfg.data(), "QTest");
    return QVariant(group.readEntry("Print Asserts")).toBool();
}

bool Settings::printSignals() const
{
    KSharedConfig::Ptr cfg = m_project->projectConfiguration();
    KConfigGroup group(cfg.data(), "QTest");
    return QVariant(group.readEntry("Print Signals")).toBool();
}

QString Settings::makeBinary() const
{
    KSharedConfig::Ptr configPtr = m_project->projectConfiguration();
    KConfigGroup builderGroup(configPtr, "MakeBuilder");
    QString makeBin = builderGroup.readEntry("Make Binary", "make");
    return makeBin;
}

KUrl Settings::cmakeProjectLibraryPath() const
{
#ifdef Q_OS_LINUX
    IBuildSystemManager* bsm = m_project->buildSystemManager();
    if (!bsm) return KUrl();
    KUrl buildDir = bsm->buildDirectory(m_project->projectItem());
    if (buildDir.isEmpty() || buildDir == KUrl("/./")) {
        return QString();
    }
    buildDir.addPath("lib");
    return buildDir;
#else
    return KUrl();
#endif
}
