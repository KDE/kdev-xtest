/*
* KDevelop xUnit integration
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

#ifndef QTEST_VERITAS_CASEBUILDER_H
#define QTEST_VERITAS_CASEBUILDER_H

#include "qxqtestexport.h"
#include <QString>

namespace QTest
{
class Executable;
class Case;

/*!
Construct a Case and it's children from the output
of a .shell QTest exe. This is an expensive operation since it
involves invoking an external executable.

@unittest QTest::Test::CaseBuilderTest
*/
class QXQTEST_EXPORT CaseBuilder
{
public:
    CaseBuilder();
    virtual ~CaseBuilder();

    /*! Set the executable which provides the Commands.
    @note mandatory to call this exactly once
    @note takes ownership */
    void setExecutable(Executable*);

    /*! Call the qtest-executable and parse it for testfunctions.
     *  May return zero. */
    virtual Case* construct();

    void setSuiteName(const QString&);

private:
    Executable* m_executable;
    QString m_suite;
};

}

#endif // QTEST_VERITAS_CASEBUILDER_H

