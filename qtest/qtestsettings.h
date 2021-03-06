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

#ifndef VERITAS_QTEST_QTESTSETTINGS
#define VERITAS_QTEST_QTESTSETTINGS

#include <QString>
#include <KUrl>
#include "qxqtestexport.h"

namespace KDevelop { class IProject; }

namespace QTest
{

/*!
 * Decouples config from implementation
 */
class QXQTEST_EXPORT ISettings
{
public:
    ISettings();
    virtual ~ISettings();

    virtual bool printAsserts() const = 0;
    virtual bool printSignals() const = 0;
    virtual QString makeBinary() const = 0;
    virtual KUrl cmakeProjectLibraryPath() const = 0;
};

/*!
 * KConfig implementation
 * TODO should connect to KConfig changed signals
 */
class Settings : public ISettings
{
public:
    explicit Settings(KDevelop::IProject*);
    virtual ~Settings();
    virtual bool printAsserts() const;
    virtual bool printSignals() const;
    virtual QString makeBinary() const;
    virtual KUrl cmakeProjectLibraryPath() const;

private:
    KDevelop::IProject* m_project;
};

} // end QTest

#endif // VERITAS_QTEST_QTESTSETTINGS
