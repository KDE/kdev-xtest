/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 * Copyright 2009 Daniel Calviño Sánchez <danxuliu@gmail.com>
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

#ifndef QXQTEST_QTESTOUTPUTPARSER
#define QXQTEST_QTESTOUTPUTPARSER

#include <QXmlStreamReader>
#include <veritas/testresult.h>
#include "qxqtestexport.h"

class QIODevice;
namespace Veritas { class Test; }

namespace QTest
{
class Case;

/*!
Interprets QTestLib executables' XML output. TestResults get reported.
It is a recoverable XML parser. It will operate on incomplete XML streams
and resume when data becomes available again.

@unittest QTest::ut::OutputParserTest
*/
class QXQTEST_EXPORT OutputParser : public QObject, public QXmlStreamReader
{
Q_OBJECT
public:
    OutputParser();
    virtual ~OutputParser();

    /*! Must be called before parsing */
    void setCase(Case*caze);

    /*! Reset internal parser state. Call this
     *  Before reusing this object. */
    void reset();

    /*! Stop processing */
    void block();

    /*! For testers only. Returns true if the parser leaked 
     *  Veritas::TestResult objects. The number of lost objects is
     *  written in @p amountLost */
    static bool fto_hasResultMemoryLeaks(int& amountLost);
    static void fto_resetResultMemoryLeakStats();

signals:
    void done();

public slots:
    /*! Start parsing the output of a testcase.
     *  Emit signals for each command being started/finished.
     *  This slot is triggered repeatedly as soon as new data is available
     *  and recovers from previous errors in the XML due to abortion. */
    void go();

private:
    // precondition
    void assertDeviceSet();
    void assertCaseSet();

     // helpers
    void processTestCaseEnd();
    void processTestFunctionStart();
    void processTestFunctionEnd();
    void processMessageStart();
    void processMessageEnd();
    void processIncidentStart();
    void processIncidentEnd();
    void processDataTagEnd();
    void processDescriptionEnd();
     
    void setDescriptionForQAssert();
    void setSuccess();

    inline bool isStartElement_(const QString& elem);
    inline bool isEndElement_(const QString& elem);
    inline bool fixtureFailed(const QString&);

    void deleteResult();
    void newResult();
    void setResult(Veritas::Test*);
    void clearResult();
    
private:
    // remember state to continue parsing
    enum State { TestCase = 0, TestFunction = 1, Incident = 2, Message = 3 };
    enum DescriptionType { Failure = 0, ExpectedFailure = 1, UnexpectedPass = 2, QSkip = 3, QAssert = 4, None = 5 };
    DescriptionType m_descriptionType;
    State m_state;
    bool m_buzzy;
    Veritas::TestResult* m_result;
    Case* m_case;
    Veritas::Test* m_cmd;
    QString m_cmdName;
    bool m_block;
    QList<Veritas::TestResult*> m_subResults;
    QString m_cdataText;
    bool m_ignoreNextIncident;

private:    // some xml constants
    static const QString c_testCase;
    static const QString c_testFunction;
    static const QString c_incident;
    static const QString c_dataTag;
    static const QString c_description;
    static const QString c_message;
    static const QString c_type;
    static const QString c_file;
    static const QString c_line;
    static const QString c_pass;
    static const QString c_fail;
    static const QString c_xfail;
    static const QString c_xpass;
    static const QString c_qfatal;
    static const QString c_skip;
    static const QString c_initTestCase;
    static const QString c_cleanupTestCase;
};

}

#endif // QXQTEST_QTESTOUTPUTPARSER
