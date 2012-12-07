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

#ifndef QXQTEST_QTESTOUTPUTPARSERTEST
#define QXQTEST_QTESTOUTPUTPARSERTEST

#include <QtTest/QtTest>
#include <veritas/testresult.h>
#include <veritas/test.h>

class QSignalSpy;

namespace QTest {
class Case;
class Command;
class OutputParser;

/*! @unitundertest QTest::OutputParser */
class OutputParserTest : public QObject
{
Q_OBJECT
public:
    /*! If @p quiet is true, the random tests will not print progress indication (dots)*/
    void setQuiet(bool quiet);

public slots:
    /*! Generates random test input and chops this into random pieces.
     *  These pieces are iterativly fed to a OutputParser. 
     *
     *  It is not executed as part of the standard suite but through a
     *  separate executable, see parserstresstest.cpp */
    void randomValidXML();

    /*! Alike startRandomTest() but this feeds random XML with randomly 
     *  placed bits of garbages in between. */
    void randomGarbageXML();

private slots:
    void initTestCase();
    void init();
    void cleanup();
    void parse_data();
    void parse();
    void initFailure();
    void cleanupFailure();
    void doubleFailure();
    void skipSingle();
    void skipAll();
    void qassert();
    void qassertx();
    void multipleResultsInSingleCommand();
    void failureAndAssertInSingleCommand();
    void expectedFailure();
    void expectedFailureAndExpectedFailureInSingleCommand();
    void expectedFailureAndFailureInSingleCommand();
    void unexpectedPass();
    void unexpectedPassAndUnexpectedPassInSingleCommand();
    void unexpectedPassAndExpectedFailureInSingleCommand();
    void multipleResultsInSingleDataDrivenCommand();
    void skipSingleAndAssertInSingleDataDrivenCommand();

private:
    struct TestInfo;

    // test data helpers
    void setupColumns();
    void addSunnyDayData();
    void addBasicFailureData();

    // custom assertions
    void assertParsed(TestInfo& testInfo);
    void assertResult(const Veritas::TestResult& expected, const Veritas::TestResult& actual);
    void assertResult(Veritas::TestResult* expected, Veritas::TestResult* actual, const QString&);
    void checkResult(TestInfo& testInfo);
    void assertNrofSubResultsEquals(int expected, Veritas::Test* t);
    void assertSubResultEquals(int nrof, Veritas::Test* test, Veritas::TestResult* expected);

    // creation methods
    Case* createTestCase(TestInfo&);
    void createTestCommand(TestInfo&, Case* parent, QString name);

    // setup helpers
    void initParser(QByteArray& xml, Case* caze);
    void setExpectedSuccess(TestInfo& tInfo);
    void setExpectedFailure(TestInfo& tInfo);
    void setExpectedResult(TestInfo& tInfo, Veritas::TestState state,
                           QString filepath, int lineNumber, QString msg);

    // random-stress test helpers
    void generateRandomInput(int maxCommands, QByteArray& qtestXmlOutput, QList<TestInfo*>&, Case*&);
    void runRandomCommand(const QList<QByteArray>& input, Case*);
    void qassertResult(Veritas::TestResult* expected, Veritas::TestResult* actual, const QString&);
    void verifyRandomResults(QList<TestInfo*>& expected);

private:
    OutputParser* m_parser;
    Case* m_caze;

    struct TestInfo
    {
        TestInfo();
        virtual ~TestInfo();
        void reset();

        Veritas::Test* test;
        QSignalSpy* started;
        QSignalSpy* finished;
        Veritas::TestResult* result;
    };

    TestInfo m_cazeInfo;
    TestInfo m_command1Info;
    TestInfo m_command2Info;

    QBuffer* m_buffer;

    enum AssertType { QTestAssert, QAssertAssert };
    AssertType m_assertType;
    QList<QByteArray> m_pieces; // current random test input
    QString m_randomTestType;
    bool m_quiet; // only used by random tests
};

}

#endif // QXQTEST_QTESTOUTPUTPARSERTEST
