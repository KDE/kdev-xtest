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

#include "qtestoutputparser.h"
#include "qtestmodelitems.h"

#include <veritas/test.h>

#include <KUrl>
#include <KDebug>

using QTest::Case;
using QTest::OutputParser;
using Veritas::Test;
using Veritas::TestResult;
using Veritas::TestState;

/*example xml:
     "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
     "<TestCase name=\"TestTest\">"
     "<Environment>"
     "<QtVersion>4.4.0-rc1</QtVersion>"
     "<QTestVersion>4.4.0-rc1</QTestVersion>"
     "</Environment>"
     "<TestFunction name=\"initTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"simpleRoot\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "<TestFunction name=\"cleanupTestCase\">"
     "<Incident type=\"pass\" file=\"\" line=\"0\" />"
     "</TestFunction>"
     "</TestCase>)"*/


const QString OutputParser::c_testCase("TestCase");
const QString OutputParser::c_testFunction("TestFunction");
const QString OutputParser::c_description("Description");
const QString OutputParser::c_incident("Incident");
const QString OutputParser::c_type("type");
const QString OutputParser::c_file("file");
const QString OutputParser::c_line("line");
const QString OutputParser::c_pass("pass");
const QString OutputParser::c_skip("skip");
const QString OutputParser::c_qfatal("qfatal");
const QString OutputParser::c_message("Message");
const QString OutputParser::c_fail("fail");
const QString OutputParser::c_xfail("xfail");
const QString OutputParser::c_xpass("xpass");
const QString OutputParser::c_initTestCase("initTestCase");
const QString OutputParser::c_cleanupTestCase("cleanupTestCase");

namespace
{
int g_result_constructed = 0;
int g_result_assigned = 0;
int g_result_destructed = 0;
}

bool OutputParser::fto_hasResultMemoryLeaks(int& amountLost)
{
    Q_ASSERT(g_result_constructed >= (g_result_assigned + g_result_destructed));
    amountLost = g_result_constructed - (g_result_assigned + g_result_destructed);
    return amountLost != 0;
}

void OutputParser::fto_resetResultMemoryLeakStats()
{
    g_result_constructed = g_result_assigned = g_result_destructed = 0;
}

void OutputParser::deleteResult()
{
    if (m_result) {
        delete m_result;
        m_result = 0;
        g_result_destructed++;
    }
}

void OutputParser::newResult()
{
    if (!m_result) {
        m_result = new TestResult;
        g_result_constructed++;
    }
}

void OutputParser::setResult(Test* test)
{
    Q_ASSERT(m_result);
    if (m_subResults.isEmpty()) {
        test->setResult(m_result);
    } else {
        TestResult* result = new TestResult;
        test->setResult(result);
        TestState state = m_result->state();
        foreach(TestResult* sub, m_subResults) {
            if (sub->state() == Veritas::RunError) {
                state = Veritas::RunError;
            } else if (sub->state() == Veritas::RunFatal) {
                state = Veritas::RunFatal;
                break;
            }
        }
        result->setState(state);
        foreach(TestResult* sub, m_subResults) {
            result->appendChild(sub);
        }
        result->appendChild(m_result);
    }
    m_result = 0;
    m_subResults.clear();
    g_result_assigned++;
}

OutputParser::OutputParser()
    : m_state(TestCase),
      m_buzzy(false),
      m_result(0),
      m_block(false),
      m_ignoreNextIncident(false)
{}

OutputParser::~OutputParser()
{
    deleteResult();
}

void OutputParser::reset()
{
    m_case = 0;
    deleteResult();
    m_result = 0;
    m_buzzy = false;
    m_state = TestCase;
    m_block = false;
    m_ignoreNextIncident = false;
}

bool OutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool OutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void OutputParser::assertDeviceSet()
{
    Q_ASSERT_X(device(), "OutputParser::go()",
               "Illegal usage. Client classes should set a QIODevice*, with setDevice().");
}

void OutputParser::assertCaseSet()
{
    Q_ASSERT_X(m_case, "OutputParser::go()",
               "Illegal usage. TestCase should have been set, with setCase().");
}

void OutputParser::setCase(Case* caze)
{
    Q_ASSERT(caze);
    m_case = caze;
}

void OutputParser::block()
{
    m_block = true;
}

void OutputParser::go()
{
    if (m_buzzy || m_block) return; // do not disturb.
    m_buzzy = true;

    assertCaseSet();
    assertDeviceSet();

    if (!device()->isOpen()) {
        device()->open(QIODevice::ReadOnly);
    }
    if (!device()->isReadable()) {
        //kWarning() << "Device not readable. Failed to parse test output.";
        m_buzzy = false;
        return;
    }

    while (!atEnd()) {
        readNext();

        if (isEndElement_(c_testCase)) {
            processTestCaseEnd();
        }
        if (isStartElement_(c_testFunction)) {
            processTestFunctionStart();
        }
        if (isEndElement_(c_testFunction)) {
            processTestFunctionEnd();
        }
        if (isStartElement_(c_message)) {
            processMessageStart();
        }
        if (isEndElement_(c_message)) {
            processMessageEnd();
        }
        if (isStartElement_(c_incident)) {
            processIncidentStart();
        }
        if (isEndElement_(c_incident)) {
            processIncidentEnd();
        }
        if (isEndElement_(c_description)) {
            processDescriptionEnd();
        }
        if (isCDATA()) {
            m_cdataText = text().toString();
        }
    }
    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();

    m_buzzy = false;
}

bool OutputParser::fixtureFailed(const QString& cmd)
{
    if (cmd != c_initTestCase && cmd != c_cleanupTestCase) {
        return false;
    } else {
        return m_result->state() != Veritas::NoResult &&
               m_result->state() != Veritas::RunSuccess;
    }
}

void OutputParser::processTestFunctionStart()
{
    if (m_state != TestCase) {
        return;
    }
    m_state = TestFunction;

    if (m_result) m_result->setState(Veritas::NoResult);
    m_cmdName = attributes().value("name").toString();
    kDebug() << m_cmdName;
    m_cmd = m_case->childNamed(m_cmdName);
    newResult();
    if (m_cmd) m_cmd->signalStarted();
}

void OutputParser::processTestCaseEnd()
{
    emit done();
}

void OutputParser::processDescriptionEnd()
{
    if (m_state != Message && m_state != Incident) {
        return;
    }
  
    if (m_state == Incident && m_ignoreNextIncident) {
        return;
    }

    if (m_descriptionType == QSkip) {
        m_result->setMessage(m_cdataText + " (skipped)");
    } else if (m_descriptionType == QAssert) {
        setDescriptionForQAssert();
    } else if (m_descriptionType == Failure || m_descriptionType == ExpectedFailure) {
        m_result->setMessage(m_cdataText);
    } else if (m_descriptionType == UnexpectedPass) {
        m_result->setMessage("Expected failure: " + m_cdataText);
    } else {
        kError() << "Unexpected state when processing the description";
    }
}

void OutputParser::setDescriptionForQAssert()
{
    // Q_ASSERT   "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/file.cpp, line 66]]></Description>\n"
    // Q_ASSERT_X "<Description><![CDATA[ASSERT failure in command: \"message\", file /path/to/file.cpp, line 66]]></Description>\n"

    int lineStart = m_cdataText.lastIndexOf(", line ");
    int lineEnd = lineStart+7;
    int fileStart = m_cdataText.lastIndexOf(" in file ");
    int fileEnd = fileStart + 9;
    int file2Start = m_cdataText.lastIndexOf(", file ");
    if (file2Start > fileStart) {
        fileStart = file2Start;
        fileEnd = file2Start + 7;
    }
    m_result->setFile(KUrl(m_cdataText.mid(fileEnd, lineStart - fileEnd)));
    m_result->setLine(m_cdataText.mid(lineEnd).toInt());
    m_result->setMessage(m_cdataText.mid(0, fileStart));
}

void OutputParser::processMessageStart()
{
    if (m_state != TestFunction) {
        return;
    }
    m_state = Message;

    QString type = attributes().value(c_type).toString();
    if (type == c_skip) {
        clearResult();
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_result->setState(Veritas::RunInfo);
        m_descriptionType = QSkip;
    } else if (type == c_qfatal) {
        clearResult();
        m_result->setState(Veritas::RunFatal);
        m_descriptionType = QAssert;
        m_ignoreNextIncident = true;
    }
}

void OutputParser::processTestFunctionEnd()
{
    if (m_state != TestFunction) {
        return;
    }
    m_state = TestCase;

    if (m_cmd) {
        setResult(m_cmd);
        m_cmd->signalFinished();
    } else if (fixtureFailed(m_cmdName)) {
        kDebug() << "init/cleanup TestCase failed";
        m_case->signalStarted();
        setResult(m_case);
        m_case->signalFinished();
    }
}

void OutputParser::clearResult()
{
    if (m_result->state() != Veritas::NoResult) { // parsed a previous result
        m_subResults << m_result;
        m_result = new TestResult;
    }
}

void OutputParser::processIncidentStart()
{
    if (m_state != TestFunction) {
        return;
    }
    m_state = Incident;
  
    if (m_ignoreNextIncident) {
        return;
    }

    QString type = attributes().value(c_type).toString();
    if (type == c_pass) {
        setSuccess();
    } else if (type == c_xfail) {
        clearResult();
        m_result->setState(Veritas::RunInfo);
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_descriptionType = ExpectedFailure;
    } else if (type == c_xpass) {
        clearResult();
        m_result->setState(Veritas::RunError);
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_descriptionType = UnexpectedPass;
    } else if (type == c_fail) {
        clearResult();
        m_result->setState(Veritas::RunError);
        m_result->setFile(KUrl(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
        m_descriptionType = Failure;
    }
}

void OutputParser::setSuccess()
{
    if (m_result->state() != Veritas::RunInfo) {
        m_result->setState(Veritas::RunSuccess);
    }
}

void OutputParser::processMessageEnd()
{
    if (m_state != Message) {
        return;
    }
    m_state = TestFunction;
}

void OutputParser::processIncidentEnd()
{
    if (m_state != Incident) {
        return;
    }
    m_state = TestFunction;

    m_ignoreNextIncident = false;
}

#include "qtestoutputparser.moc"
