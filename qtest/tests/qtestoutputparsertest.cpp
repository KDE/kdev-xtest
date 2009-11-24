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

#include "qtestoutputparsertest.h"
#include "../qtestoutputparser.h"
#include "../qtestmodelitems.h"
#include "../qtestmodelitems.h"

#include "kdevtest.h"

#include <QBuffer>
#include <QMetaType>
#include <QFileInfo>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractListModel>

using QTest::OutputParser;
using QTest::Case;
using QTest::Command;
using Veritas::TestResult;

using QTest::OutputParserTest;

Q_DECLARE_METATYPE(QFileInfo)
Q_DECLARE_METATYPE(Veritas::TestState)
Q_DECLARE_METATYPE(QTest::Case*)
Q_DECLARE_METATYPE(QModelIndex)

/*example xml output :
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

//////////////// HELPER MACROS ///////////////////////////////////////////////

#define QTEST_HEADER_XML \
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"\
    "<TestCase name=\"TestTest\">\n"\
    "<Environment>\n"\
        "<QtVersion>4.4.0-rc1</QtVersion>\n"\
        "<QTestVersion>4.4.0-rc1</QTestVersion>\n"\
    "</Environment>\n"

#define QTEST_INITTESTCASE_XML\
    "<TestFunction name=\"initTestCase\">\n"\
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"\
    "</TestFunction>\n"

#define QTEST_CLEANUPTESTCASE_XML\
    "<TestFunction name=\"cleanupTestCase\">\n"\
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"\
    "</TestFunction>\n"

#define QTEST_FOOTER_XML \
    "</TestCase>\n"

#define QTEST_SUCCESSFUNCTION_XML \
    "<TestFunction name=\"someCommand\">\n" \
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n" \
    "</TestFunction>\n"

#include <QSignalSpy>

//////////////// HELPERS /////////////////////////////////////////////////////

namespace QTest
{
template<> inline char* toString(const TestResult& res)
{
    return qstrdup((QString::number(res.state()) + ' ' +
                    res.message()).toLatin1().constData());
}
}

////////////// FIXTURE ///////////////////////////////////////////////////////

void OutputParserTest::initTestCase()
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

OutputParserTest::TestInfo::TestInfo()
    : test(0), started(0), finished(0), result(0)
{}

OutputParserTest::TestInfo::~TestInfo()
{
    if (started) delete started;
    if (finished) delete finished;
    if (result) delete result;
}

void OutputParserTest::TestInfo::reset()
{
    if (started) delete started;
    if (finished) delete finished;
    if (result) delete result;
    test = 0;
    started = 0;
    finished = 0;
    result = 0;
}


void OutputParserTest::init()
{
    m_buffer = 0;
    m_parser = new OutputParser;
    m_caze = createTestCase(m_cazeInfo);
    m_assertType = QTestAssert;
}

void OutputParserTest::cleanup()
{
    delete m_parser;
    delete m_buffer;
    delete m_caze;
    m_cazeInfo.reset();
    m_command1Info.reset();
    m_command2Info.reset();
}

// fixture setup helper
void OutputParserTest::initParser(QByteArray& xml, Case* caze)
{
    m_buffer = new QBuffer(&xml, 0);
    m_parser->setDevice(m_buffer);
    m_parser->setCase(caze);
    m_buffer->open(QIODevice::ReadOnly);
}



// creation method
Case* OutputParserTest::createTestCase(TestInfo& cInfo)
{
    Case* caze = new Case("TestTest", QFileInfo(), 0);
    cInfo.test = caze;
    cInfo.finished = new QSignalSpy(caze, SIGNAL(finished(QModelIndex)));
    cInfo.started  = new QSignalSpy(caze, SIGNAL(started(QModelIndex)));
    return caze;
}

// creation method
void OutputParserTest::createTestCommand(TestInfo& cInfo, Case* parent, QString name)
{
    cInfo.test = new Command(name, (Case*)parent);
    parent->addChild(cInfo.test);
    cInfo.started  = new QSignalSpy(cInfo.test, SIGNAL(started(QModelIndex)));
    cInfo.finished = new QSignalSpy(cInfo.test, SIGNAL(finished(QModelIndex)));
}

// fixture setup helper
void OutputParserTest::setExpectedSuccess(TestInfo& tInfo)
{
    setExpectedResult(tInfo, Veritas::RunSuccess, "", 0, "");
}

// fixture setup helper
void OutputParserTest::setExpectedFailure(TestInfo& tInfo)
{
    setExpectedResult(tInfo, Veritas::RunError, "/path/to/file.cpp",
                             100, "failure message");
}

// fixture setup helper
void OutputParserTest::setExpectedResult(
    TestInfo& tInfo,
    Veritas::TestState state,
    QString filePath,
    int lineNumber,
    QString failureMessage)
{
    TestResult* res = new TestResult;
    res->setState(state);
    res->setFile(filePath);
    res->setLine(lineNumber);
    res->setMessage(failureMessage);
    tInfo.result = res;
}

/////////////// DATA TEST ///////////////////////////////////////////////////////////

//test command
void OutputParserTest::parse()
{
    // exercise
    QFETCH(QByteArray, xml);
    initParser(xml, m_caze);

    QFETCH(Veritas::TestState, state);
    QFETCH(QFileInfo, file);
    QFETCH(int, line);
    QFETCH(QString, message);
    createTestCommand(m_command1Info, m_caze, "someCommand");
    setExpectedResult(m_command1Info, state, file.filePath(), line, message);
    QSignalSpy spy(m_parser, SIGNAL(done()));

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
    KOMPARE(1, spy.count());
}

// test data implementation
void OutputParserTest::parse_data()
{
    setupColumns();
    addSunnyDayData();
    addBasicFailureData();
}

// helper for parse_data
void OutputParserTest::setupColumns()
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<Veritas::TestState>("state");
    QTest::addColumn<QFileInfo>("file");
    QTest::addColumn<int>("line");
    QTest::addColumn<QString>("message");
    QTest::addColumn<Case*>("case");
}

// test data
void OutputParserTest::addSunnyDayData()
{
    // first row - sunny day test succes
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"someCommand\">\n"
            "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;

    QTest::newRow("sunny day test succes")
    << input << Veritas::RunSuccess
    << QFileInfo("") << 0 << "" << m_caze;
}

// test data
void OutputParserTest::addBasicFailureData()
{
    // second row - test failure
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"someCommand\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[some message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;

    QTest::newRow("basic failure")
    << input << Veritas::RunError
    << QFileInfo("/path/to/file.cpp") << 100
    << "some message" << m_caze;
}

////////////// TEST COMMANDS ///////////////////////////////////////////////////////

// test command
void OutputParserTest::initFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        "<TestFunction name=\"initTestCase\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_SUCCESSFUNCTION_XML
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);
    setExpectedFailure(m_cazeInfo);

    m_parser->go();

    assertParsed(m_cazeInfo);
    checkResult(m_cazeInfo);
}

// test command
void OutputParserTest::cleanupFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        QTEST_SUCCESSFUNCTION_XML
        "<TestFunction name=\"cleanupTestCase\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    setExpectedFailure(m_cazeInfo);
    createTestCommand(m_command1Info, m_caze, "someCommand");
    setExpectedSuccess(m_command1Info);

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
    assertParsed(m_cazeInfo);
    checkResult(m_cazeInfo);
}

// command
void OutputParserTest::doubleFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command1\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        "<TestFunction name=\"command2\">\n"
            "<Incident type=\"fail\" file=\"/path/to/another.cpp\" line=\"50\">\n"
            "<Description><![CDATA[another failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command1");
    setExpectedResult(m_command1Info, Veritas::RunError, "/path/to/file.cpp",
                              100, "failure message");
    createTestCommand(m_command2Info, m_caze, "command2");
    setExpectedResult(m_command2Info, Veritas::RunError, "/path/to/another.cpp",
                               50, "another failure message");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
    assertParsed(m_command2Info);
    checkResult(m_command2Info);
}

// test command
void OutputParserTest::skipSingle()
{
    // QSKIP with the SkipSingle flag
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"skip\" file=\"/path/to/file.cpp\" line=\"8\">\n"
        "    <Description><![CDATA[skipCommand]]></Description>\n"
        "</Message>\n"
        "<Incident type=\"pass\" file=\"\" line=\"0\" />\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);
    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunInfo, "/path/to/file.cpp", 8, "skipCommand (skipped)");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void OutputParserTest::skipAll()
{
    // QSKIP with the SkipAll flag. QTestLib refuses to print an <Incident type="pass" /> for those.
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"skip\" file=\"/path/to/file.cpp\" line=\"8\">\n"
        "    <Description><![CDATA[skipCommand]]></Description>\n"
        "</Message>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunInfo, "/path/to/file.cpp", 8, "skipCommand (skipped)");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void OutputParserTest::qassert()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/file.cpp, line 66]]></Description>\n"
        "</Message>\n"
        "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
            "<Description><![CDATA[Received a fatal error.]]></Description>\n"
        "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunFatal, "/path/to/file.cpp", 66, "ASSERT: \"condition\"");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void OutputParserTest::qassertx()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
        "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[ASSERT failure in command: \"message in file\", file /path/to/file.cpp, line 66]]></Description>\n"
        "</Message>\n"
        "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
            "<Description><![CDATA[Received a fatal error.]]></Description>\n"
        "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunFatal, "/path/to/file.cpp", 66, "ASSERT failure in command: \"message in file\"");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void OutputParserTest::multipleResultsInSingleCommand()
{
    // More than one failure messages in a single test function.
    // The parser should add these as individual sub-TestResults on 
    // the Command's TestResult
    // You can trigger this behaviour in QTestLib with QVERIFY/QCOMPARES in different methods.

    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
            "<Incident type=\"fail\" file=\"/path/to/file2.cpp\" line=\"50\">\n"
            "<Description><![CDATA[another failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    TestResult* result1 = new TestResult();
    result1->setMessage("failure message");
    result1->setLine(100);
    result1->setFile(KUrl("/path/to/file.cpp"));
    result1->setState(Veritas::RunError);

    TestResult* result2 = new TestResult();
    result2->setMessage("another failure message");
    result2->setLine(50);
    result2->setFile(KUrl("/path/to/file2.cpp"));
    result2->setState(Veritas::RunError);

    m_parser->go();

    assertParsed(m_command1Info);
    KOMPARE(Veritas::RunError, m_command1Info.test->state()); // overal test state should be RunError
    assertNrofSubResultsEquals(2, m_command1Info.test);
    assertSubResultEquals(0, m_command1Info.test, result1);
    assertSubResultEquals(1, m_command1Info.test, result2);

    delete result1; delete result2;
}

// test command
void OutputParserTest::failureAndAssertInSingleCommand()
{
    // More than one failure messages in a single test function.
    // The parser should add these as individual sub-TestResults on 
    // the Command's TestResult
    // You can trigger this behaviour in QTestLib with QVERIFY/QCOMPARES in different methods.

    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
            "<Incident type=\"fail\" file=\"/path/to/file2.cpp\" line=\"50\">\n"
            "<Description><![CDATA[another failure message]]></Description>\n"
            "</Incident>\n"
            "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
            "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/file.cpp, line 66]]></Description>\n"
            "</Message>\n"
            "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
            "<Description><![CDATA[Received a fatal error.]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    TestResult* result1 = new TestResult();
    result1->setMessage("failure message");
    result1->setLine(100);
    result1->setFile(KUrl("/path/to/file.cpp"));
    result1->setState(Veritas::RunError);

    TestResult* result2 = new TestResult();
    result2->setMessage("another failure message");
    result2->setLine(50);
    result2->setFile(KUrl("/path/to/file2.cpp"));
    result2->setState(Veritas::RunError);

    TestResult* result3 = new TestResult;
    result3->setMessage("ASSERT: \"condition\"");
    result3->setState(Veritas::RunFatal);
    result3->setFile(KUrl("/path/to/file.cpp"));
    result3->setLine(66);

    m_parser->go();

    assertParsed(m_command1Info);
    KOMPARE(Veritas::RunError, m_command1Info.test->state()); // overal test state should be RunError
    assertNrofSubResultsEquals(3, m_command1Info.test);
    assertSubResultEquals(0, m_command1Info.test, result1);
    assertSubResultEquals(1, m_command1Info.test, result2);
    assertSubResultEquals(2, m_command1Info.test, result3);

    delete result1; delete result2; delete result3;
}

// test command
void OutputParserTest::expectedFailure()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"xfail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[expected failure comment]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunInfo, "/path/to/file.cpp", 100, "expected failure comment");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void OutputParserTest::expectedFailureAndExpectedFailureInSingleCommand()
{
    // More than one expected failure in a single test function.
    // The parser should add these as individual sub-TestResults on 
    // the Command's TestResult
    // You can trigger this behaviour in QTestLib with several QEXPECT_FAIL in
    // the same method.

    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"xfail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[expected failure comment]]></Description>\n"
            "</Incident>\n"
            "<Incident type=\"xfail\" file=\"/path/to/file.cpp\" line=\"103\">\n"
            "<Description><![CDATA[another expected failure comment]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    TestResult result1;
    result1.setMessage("expected failure comment");
    result1.setLine(100);
    result1.setFile(KUrl("/path/to/file.cpp"));
    result1.setState(Veritas::RunInfo);

    TestResult result2;
    result2.setMessage("another expected failure comment");
    result2.setLine(103);
    result2.setFile(KUrl("/path/to/file.cpp"));
    result2.setState(Veritas::RunInfo);

    m_parser->go();

    assertParsed(m_command1Info);
    KOMPARE(Veritas::RunInfo, m_command1Info.test->state()); // overal test state should be RunInfo
    assertNrofSubResultsEquals(2, m_command1Info.test);
    assertSubResultEquals(0, m_command1Info.test, &result1);
    assertSubResultEquals(1, m_command1Info.test, &result2);
}

// test command
void OutputParserTest::expectedFailureAndFailureInSingleCommand()
{
    // A failure after an expected failure in a single test function.
    // The parser should add these as individual sub-TestResults on 
    // the Command's TestResult
    // You can trigger this behaviour in QTestLib with QVERIFY/QCOMPARE after
    // the QVERIFY/QCOMPARE expected to fail by QEXPECT_FAIL.
    
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"xfail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[expected failure comment]]></Description>\n"
            "</Incident>\n"
            "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"103\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    TestResult result1;
    result1.setMessage("expected failure comment");
    result1.setLine(100);
    result1.setFile(KUrl("/path/to/file.cpp"));
    result1.setState(Veritas::RunInfo);

    TestResult result2;
    result2.setMessage("failure message");
    result2.setLine(103);
    result2.setFile(KUrl("/path/to/file.cpp"));
    result2.setState(Veritas::RunError);

    m_parser->go();

    assertParsed(m_command1Info);
    KOMPARE(Veritas::RunError, m_command1Info.test->state()); // overal test state should be RunError
    assertNrofSubResultsEquals(2, m_command1Info.test);
    assertSubResultEquals(0, m_command1Info.test, &result1);
    assertSubResultEquals(1, m_command1Info.test, &result2);
}

// test command
void OutputParserTest::unexpectedPass()
{
    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"xpass\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    setExpectedResult(m_command1Info, Veritas::RunError, "/path/to/file.cpp", 100, "Expected failure: failure message");

    m_parser->go();

    assertParsed(m_command1Info);
    checkResult(m_command1Info);
}

// test command
void OutputParserTest::unexpectedPassAndUnexpectedPassInSingleCommand()
{
    // More than one unexpected pass in a single test function.
    // The parser should add these as individual sub-TestResults on 
    // the Command's TestResult
    // You can trigger this behaviour in QTestLib with several QEXPECT_FAIL
    // using "Continue" mode in the same method.

    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"xpass\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
            "<Incident type=\"xpass\" file=\"/path/to/file.cpp\" line=\"103\">\n"
            "<Description><![CDATA[another failure message]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    TestResult result1;
    result1.setMessage("Expected failure: failure message");
    result1.setLine(100);
    result1.setFile(KUrl("/path/to/file.cpp"));
    result1.setState(Veritas::RunError);

    TestResult result2;
    result2.setMessage("Expected failure: another failure message");
    result2.setLine(103);
    result2.setFile(KUrl("/path/to/file.cpp"));
    result2.setState(Veritas::RunError);

    m_parser->go();

    assertParsed(m_command1Info);
    KOMPARE(Veritas::RunError, m_command1Info.test->state()); // overal test state should be RunError
    assertNrofSubResultsEquals(2, m_command1Info.test);
    assertSubResultEquals(0, m_command1Info.test, &result1);
    assertSubResultEquals(1, m_command1Info.test, &result2);
}

// test command
void OutputParserTest::unexpectedPassAndExpectedFailureInSingleCommand()
{
    // An expected failure after an unexpected pass in a single test function.
    // The parser should add these as individual sub-TestResults on 
    // the Command's TestResult
    // You can trigger this behaviour in QTestLib with QEXPECT_FAIL using
    // "Continue" followed by another QEXPECT_FAIL in the same method.

    QByteArray input =
        QTEST_HEADER_XML
        QTEST_INITTESTCASE_XML
        "<TestFunction name=\"command\">\n"
            "<Incident type=\"xpass\" file=\"/path/to/file.cpp\" line=\"100\">\n"
            "<Description><![CDATA[failure message]]></Description>\n"
            "</Incident>\n"
            "<Incident type=\"xfail\" file=\"/path/to/file.cpp\" line=\"103\">\n"
            "<Description><![CDATA[expected failure comment]]></Description>\n"
            "</Incident>\n"
        "</TestFunction>\n"
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;
    initParser(input, m_caze);

    createTestCommand(m_command1Info, m_caze, "command");
    TestResult result1;
    result1.setMessage("Expected failure: failure message");
    result1.setLine(100);
    result1.setFile(KUrl("/path/to/file.cpp"));
    result1.setState(Veritas::RunError);

    TestResult result2;
    result2.setMessage("expected failure comment");
    result2.setLine(103);
    result2.setFile(KUrl("/path/to/file.cpp"));
    result2.setState(Veritas::RunInfo);

    m_parser->go();

    assertParsed(m_command1Info);
    KOMPARE(Veritas::RunError, m_command1Info.test->state()); // overal test state should be RunError
    assertNrofSubResultsEquals(2, m_command1Info.test);
    assertSubResultEquals(0, m_command1Info.test, &result1);
    assertSubResultEquals(1, m_command1Info.test, &result2);
}


void OutputParserTest::assertNrofSubResultsEquals(int expected, Veritas::Test* t)
{
    KVERIFY(t->result());
    KOMPARE(expected, t->result()->childCount());
    for (int i=0; i<expected; i++) {
        KVERIFY(t->result()->child(i));
    }
}

void OutputParserTest::assertSubResultEquals(int nrof, Veritas::Test* test, TestResult* expected)
{
    KVERIFY(test);
    KVERIFY(test->result());
    KVERIFY(test->result()->childCount() > nrof);
    TestResult* actual = test->result()->child(nrof);
    KVERIFY(actual);
    KOMPARE(expected->message(), actual->message());
    KOMPARE(expected->line(), actual->line());
    KOMPARE(expected->state(), actual->state());
    KOMPARE(expected->file(), actual->file());
}

/////////////// RANDOM TEST ///////////////////////////////////////////////////

#include <stdlib.h>
#include <time.h>
#include <QTextStream>

namespace {
QTextStream cout(stdout);
}

QList<QByteArray> chopInPieces(const QByteArray& input, int nrofPieces)
{
    QList<int> cutPositions;
    for (int i=0; i<nrofPieces-1; i++) {
        int pos = -1;
        do {
            pos = rand() % input.size();
        } while (cutPositions.contains(pos));
        cutPositions << pos;
    }
    qSort(cutPositions);
    QList<QByteArray> pieces;
    pieces << input.mid(0, (nrofPieces == 1) ? -1 : cutPositions[0]);
    for (int i=0; i<nrofPieces-1; i++) {
        int startPos = -1;
        int nrofChars = -1;
        if (i==nrofPieces-2) {
            startPos = cutPositions[i];
            nrofChars = -1;
        } else {
            startPos = cutPositions[i];
            nrofChars = cutPositions[i+1] - cutPositions[i];
        }
        pieces << input.mid(startPos, nrofChars);
    }
    return pieces;
}

void OutputParserTest::runRandomCommand(const QList<QByteArray>& pieces, Case* caze)
{
    m_parser = new OutputParser;
    m_buffer = new QBuffer();
    m_parser->setDevice(m_buffer);
    m_parser->setCase(caze);
    m_buffer->open(QIODevice::ReadOnly | QIODevice::WriteOnly);

    foreach(const QByteArray& piece, pieces) {
        qint64 pos = m_buffer->pos();
        m_buffer->write(piece);
        m_buffer->seek(pos);
        m_parser->go();
    }

    delete m_parser;
    delete m_buffer;
}

void OutputParserTest::verifyRandomResults(QList<TestInfo*>& expected)
{
    foreach(TestInfo* ti, expected) {
        assertParsed(*ti);
        checkResult(*ti);
    }
}

void OutputParserTest::generateRandomInput(int maxCommands, QByteArray& parserInput, QList<TestInfo*>& expected, Case*& caze)
{
    parserInput = QTEST_HEADER_XML
                     QTEST_INITTESTCASE_XML;

    TestInfo* cazeInfo = new TestInfo;
    caze = createTestCase(*cazeInfo);
    delete cazeInfo;
    int nrofCommands = (rand() % maxCommands) + 1;

    int nrofSuccess = 0, nrofFailures = 0, nrofSkipSingle = 0, nrofSkipAll = 0, nrofAsserts = 0, nrofExpectedFailures = 0, nrofUnexpectedPasses = 0;
    for (int i=0; i<nrofCommands; i++) {
        int type = rand() % 9;
        TestInfo* cmdInfo = new TestInfo;
        createTestCommand(*cmdInfo, caze, QString("command%1").arg(i));
        parserInput += "<TestFunction name=\"command" + QString::number(i).toLatin1() + "\">\n";
        switch(type) {
        case 0: case 1: // run-off-the-mill success
            setExpectedResult(*cmdInfo, Veritas::RunSuccess, "", 0, "");
            parserInput +=
                "<Incident type=\"pass\" file=\"\" line=\"0\" />\n";
            nrofSuccess++;
            break;
        case 2: case 3: // normal failure
            setExpectedResult(*cmdInfo, Veritas::RunError, "/path/to/file.cpp", 100, "some message");
            parserInput +=
                "<Incident type=\"fail\" file=\"/path/to/file.cpp\" line=\"100\">\n"
                    "<Description><![CDATA[some message]]></Description>\n"
                "</Incident>\n";
            nrofFailures++;
            break;
        case 4: // SkipSingle
            setExpectedResult(*cmdInfo, Veritas::RunInfo, "/path/to/skipfile.cpp", 8, "skipCommand (skipped)");
            parserInput +=
                "<Message type=\"skip\" file=\"/path/to/skipfile.cpp\" line=\"8\">\n"
                "    <Description><![CDATA[skipCommand]]></Description>\n"
                "</Message>\n"
                "<Incident type=\"pass\" file=\"\" line=\"0\" />\n";
            nrofSkipSingle++;
            break;
        case 5: // SkipAll ie without <Incident type='pass' />
            setExpectedResult(*cmdInfo, Veritas::RunInfo, "/path/to/skipfile.cpp", 9, "skipCommand (skipped)");
            parserInput +=
                "<Message type=\"skip\" file=\"/path/to/skipfile.cpp\" line=\"9\">\n"
                "    <Description><![CDATA[skipCommand]]></Description>\n"
                "</Message>\n";
            nrofSkipAll++;
            break;
        case 6: // Q_ASSERT
            setExpectedResult(*cmdInfo, Veritas::RunFatal, "/path/to/qassertfile.cpp", 66, "ASSERT: \"condition\"");
            parserInput +=
                "<Message type=\"qfatal\" file=\"\" line=\"0\">\n"
                    "<Description><![CDATA[ASSERT: \"condition\" in file /path/to/qassertfile.cpp, line 66]]></Description>\n"
                "</Message>\n"
                "<Incident type=\"fail\" file=\"Unknown file\" line=\"0\">\n"
                    "<Description><![CDATA[Received a fatal error.]]></Description>\n"
                "</Incident>\n";
            nrofAsserts++;
            break;
        case 7: // expected failure
            setExpectedResult(*cmdInfo, Veritas::RunInfo, "/path/to/xfailfile.cpp", 23, "expected failure comment");
            parserInput +=
                "<Incident type=\"xfail\" file=\"/path/to/xfailfile.cpp\" line=\"23\">\n"
                    "<Description><![CDATA[expected failure comment]]></Description>\n"
                "</Incident>\n";
            nrofExpectedFailures++;
            break;
        case 8: // unexpected pass
            setExpectedResult(*cmdInfo, Veritas::RunError, "/path/to/xpassfile.cpp", 42, "Expected failure: failure message");
            parserInput +=
                "<Incident type=\"xpass\" file=\"/path/to/xpassfile.cpp\" line=\"42\">\n"
                    "<Description><![CDATA[failure message]]></Description>\n"
                "</Incident>\n";
            nrofUnexpectedPasses++;
            break;
        default: ;
        }
        parserInput += "</TestFunction>\n";
        expected << cmdInfo;
    }

    parserInput +=
        QTEST_CLEANUPTESTCASE_XML
        QTEST_FOOTER_XML;

    m_randomTestType += "[success:" + QString::number(nrofSuccess) + ";failures:" + QString::number(nrofFailures) +
                        ";skipsingle:" + QString::number(nrofSkipSingle) + ";skipall:" + QString::number(nrofSkipAll) +
                        ";asserts:" + QString::number(nrofAsserts) + ";expected failures:" + QString::number(nrofExpectedFailures) +
                        ";unexpected passes:" + QString::number(nrofUnexpectedPasses) + "]\n";
}

namespace {

QByteArray randomWord(int length)
{
    static QByteArray letters = "abcdefghijklmnopqrstuvwxyz1234567890abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-+=__[]{}'\"|,./<><><><><><><><><><><><><><><><><><><><><><><><><><><>\"\"\"\"\"\"?";
    static int nrof = letters.size();

    QByteArray result;
    result.reserve(length);
    for (int i=0; i<length; i++) {
        result.append(letters[rand() % nrof]);
    }
    return result;
}

void insertGarbage(QList<QByteArray>& input)
{
    QMutableListIterator<QByteArray> it(input);
    while(it.hasNext()) {
        int i = rand() % 3;
        if (i==2) {
            int length = (rand() % 20) +1;
            QByteArray garbage = randomWord(length);
            it.insert(garbage);
        }
        it.next();
    }
}
}

void OutputParserTest::setQuiet(bool quiet)
{
    m_quiet = quiet;
}

void OutputParserTest::randomValidXML()
{
    cout << "------------- OutputParser Random Stress Test  -------------" << endl;
    srand(time(0));

    int count = 0;
    m_assertType = QAssertAssert;
    for (int nrofPieces=1; nrofPieces<31; nrofPieces++) {
        for (int i=0; i<1000; i++) {
            m_randomTestType = "[pieces:" + QString::number(nrofPieces) + ";cycle:" + QString::number(i) + "]\n";
            QByteArray input;
            Case* caze;
            QList<TestInfo*> expected;

            generateRandomInput(30, input, expected, caze);
            if (input.size() / 2 < nrofPieces) continue;
            m_pieces = chopInPieces(input, nrofPieces);
            runRandomCommand(m_pieces, caze);
            verifyRandomResults(expected);
            if (!m_quiet) {
                cout << ".";
                cout.flush();
            }
            count++;

            delete caze;
            qDeleteAll(expected);
        }
    }

    cout << "\n" << count << " tests succesful!" << endl;
    cout << "------------- OutputParser Random Stress Test  -------------" << endl;
    QCoreApplication::quit();
}

void OutputParserTest::randomGarbageXML()
{
    cout << "------------- OutputParser Random Garbage Test -------------" << endl;
    srand(time(0));

    int count = 0;
    m_assertType = QAssertAssert;

    for (int nrofPieces=1; nrofPieces<31; nrofPieces++) {
        for (int i=0; i<1000; i++) {
            m_randomTestType = "[pieces:" + QString::number(nrofPieces) + ";cycle:" + QString::number(i) + "]\n";
            QByteArray input;
            Case* caze;
            QList<TestInfo*> expected;

            generateRandomInput(30, input, expected, caze);
            if (input.size() / 2 < nrofPieces) continue;
            m_pieces = chopInPieces(input, nrofPieces);
            insertGarbage(m_pieces);
            runRandomCommand(m_pieces, caze);
            if (!m_quiet) {
                cout << ".";
                cout.flush();
            }
            count++;

            delete caze;
            qDeleteAll(expected);
        }
    }

    cout << "\n" << count << " tests succesful!" << endl;
    cout << "------------- OutputParser Random Garbage Test -------------" << endl;
    QCoreApplication::quit();
}

/////////////// CUSTOM ASSERTIONS ////////////////////////////////////////////

#include <veritas/test.h>
using Veritas::Test;

// custom assertion
void OutputParserTest::checkResult(TestInfo& testInfo)
{
    Veritas::Test* test = testInfo.test;
    TestResult* actual = test->result();
    TestResult* expected = testInfo.result;
    assertResult(expected, actual, test->name());
}

namespace {
QByteArray join(const QList<QByteArray>& list)
{
    QByteArray joined;
    foreach(const QByteArray& item, list) {
        joined = joined.append("-------------------\n").append(item).append('\n');
    }
    return joined;
}
}

#define Q_ASSERT_GOT_SIGNAL(spy, test) \
if (1 != spy->count()) {\
    Q_ASSERT_X(1 == spy->count(), "assertParsed()", \
               QString("Parser did not emit " #spy " signal for %1.\n%3 input:\n%2").\
               arg(test->name()).arg(join(m_pieces).constData()).arg(m_randomTestType).toLatin1().constData()); \
} else (void)(0)

#define Q_ASSERT_EQUALS(expected, actual, item)\
if (expected != actual) {\
    Q_ASSERT_X(expected == actual, "assertResult", \
                QString("Wrong test " #item ". Expected %1 got %2.\nTest: %5\n%4\ninput:\n%3").\
                arg(expected).arg(actual).arg(join(m_pieces).data()).arg(m_randomTestType).arg(testName).toLatin1().constData());\
} else (void)(0)


// helper
void OutputParserTest::assertParsed(TestInfo& testInfo)
{
    QSignalSpy* started = testInfo.started;
    QSignalSpy* completed = testInfo.finished;
    Veritas::Test* test = testInfo.test;

    if (m_assertType == QTestAssert) {
        KOMPARE_MSG(1, started->count(),
            QString("OutputParser did not emit started signal for ") + test->name());
        KOMPARE_MSG(1, completed->count(),
            QString("OutputParser did not emit finished signal for ") + test->name());
    } else if (m_assertType == QAssertAssert) {
        Q_ASSERT_GOT_SIGNAL(started, test);
        Q_ASSERT_GOT_SIGNAL(completed, test);
    } else { Q_ASSERT(0); }
}

void OutputParserTest::assertResult(TestResult* expected, TestResult* actual, const QString& name)
{
    if (m_assertType == QTestAssert) {
        assertResult(*expected, *actual);
    } else if (m_assertType == QAssertAssert) {
        qassertResult(expected, actual, name);
    }
}

// helper
void OutputParserTest::assertResult(const TestResult& expected, const TestResult& actual)
{
    KOMPARE_MSG(expected.state(), actual.state(),
                "Expected " + QString::number(expected.state()) +
                " got " + QString::number(actual.state()));
    KOMPARE(expected.line(), actual.line());
    KOMPARE(expected.file(), actual.file());
    KOMPARE_MSG(expected.message(), actual.message(),
                "Expected " + expected.message() + " got " + actual.message());
}

// helper
void OutputParserTest::qassertResult(TestResult* expected, TestResult* actual, const QString& testName)
{
    Q_ASSERT_EQUALS(expected->state(), actual->state(), state);
    Q_ASSERT_EQUALS(expected->file().toLocalFile(), actual->file().path(), file);
    Q_ASSERT_EQUALS(expected->line(), actual->line(), line);
    Q_ASSERT_EQUALS(expected->message(), actual->message(), message);
}

#include "qtestoutputparsertest.moc"
// QTEST_KDEMAIN(OutputParserTest, NoGUI)
