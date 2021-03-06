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

#include "qtestrunnertest.h"
#include <kasserts.h>
#include <qtest_kde.h>

#include <KDebug>
#include <QBuffer>
#include <KLocale>

#include "../xmlregister.h"
#include "../qtestmodelitems.h"
#include "../qtestoutputparser.h"
#include "../executable.h"
#include "../qtestsettings.h"

#include <veritas/test.h>
#include <veritas/testresult.h>
#include <veritas/tests/runnertesthelper.h>

using Veritas::RunnerTestHelper;
using QTest::QTestRunnerTest;
using namespace QTest;

namespace
{
class FakeSettings : public ISettings
{
    virtual bool printAsserts() const { return false; }
    virtual bool printSignals() const { return false; }
    virtual QString makeBinary() const { return QString(); }
    virtual KUrl cmakeProjectLibraryPath() const { return KUrl(); }
};
}

Q_DECLARE_METATYPE(QList<QStringList>)

void QTestRunnerTest::init()
{
    m_settings = new FakeSettings;
    OutputParser::fto_resetResultMemoryLeakStats();
    m_runner = new RunnerTestHelper;
    m_runner->initializeGUI();
}

void QTestRunnerTest::cleanup()
{
    m_runner->cleanupGUI();
    delete m_runner;
    delete m_settings;

    int nrofLeaks = 0;
    bool foundLeaks = OutputParser::fto_hasResultMemoryLeaks(nrofLeaks);
    QString errorMsg = QString("OutputParser leaked %1 number of Veritas::TestResult's.").arg(nrofLeaks);
    KVERIFY_MSG(!foundLeaks, errorMsg);
}

// command
void QTestRunnerTest::empty()
{
    QByteArray reg =
            "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
            "<root dir=\"\">\n"
            "</root>\n";
    Veritas::Test* root = fetchRoot(reg);

    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList runnerItems;
    runnerItems << "0 x";
    m_runner->verifyTestTree(runnerItems);

    QList<QStringList> results;
    m_runner->verifyResultItems(results);
}

QByteArray sunnyDayXml =
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
    "<root dir=\"\">\n"
    "<suite name=\"suite1\" dir=\"suite1\">\n"
    "<case name=\"fakeqtest1\" exe=\"fakeqtest1\">\n"
    "<command name=\"cmd1\" />\n"
    "<command name=\"cmd2\" />\n"
    "</case>\n"
    "<case name=\"fakeqtest2\" exe=\"fakeqtest2\">\n"
    "<command name=\"cmd1\" />\n"
    "<command name=\"cmd2\" />\n"
    "</case>"
    "<case name=\"fakeqtest3\" exe=\"fakeqtest3\">\n"
    "<command name=\"cmd1\" />\n"
    "</case>\n"
    "</suite>\n"
    "</root>\n";

QStringList sunnyDayTests()
{
    QStringList runnerItems;
    runnerItems
            << "0 suite1"
            << "0 0 fakeqtest1"
            << "0 0 0 cmd1"
            << "0 0 1 cmd2"
            << "0 0 2 x"
            << "0 1 fakeqtest2"
            << "0 1 0 cmd1"
            << "0 1 1 cmd2"
            << "0 1 2 x"
            << "0 2 fakeqtest3"
            << "0 2 0 cmd1"
            << "0 2 1 x"
            << "0 3 x"
            << "1 x";
    return runnerItems;
}

QList<QStringList> sunnyDayResultItems()
{
    QStringList result;
    result << "cmd2" << "failure message" << "fakeqtest2.cpp" << "2";
    return QList<QStringList>() << result;
}

QMap<QString, Veritas::TestState> sunnyDayTestStates()
{
    QMap<QString, Veritas::TestState> states;
    states["suite1/fakeqtest1/cmd1"] = Veritas::RunSuccess;
    states["suite1/fakeqtest1/cmd2"] = Veritas::RunSuccess;
    states["suite1/fakeqtest2/cmd1"] = Veritas::RunSuccess;
    states["suite1/fakeqtest2/cmd2"] = Veritas::RunError;
    states["suite1/fakeqtest3/cmd1"] = Veritas::RunSuccess;
    return states;
}

// command
void QTestRunnerTest::sunnyDay()
{
    Veritas::Test* root = fetchRoot(sunnyDayXml);
    m_runner->setRoot(root);
    m_runner->runTests();

    m_runner->verifyTestTree(sunnyDayTests());
    m_runner->verifyResultItems(sunnyDayResultItems());
    m_runner->verifyTestStates(sunnyDayTestStates(), root);
    assertAllFilesClosed(root);
}

// command
void QTestRunnerTest::runTwice()
{
    Veritas::Test* root = fetchRoot(sunnyDayXml);
    m_runner->setRoot(root);
    m_runner->runTests();
    m_runner->runTests();

    m_runner->verifyTestTree(sunnyDayTests());
    m_runner->verifyResultItems(sunnyDayResultItems());
    assertAllFilesClosed(root);
}

QByteArray singleGreenCommandXml =
    "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
    "<root dir=\"\">\n"
    "<suite name=\"suite1\" dir=\"suite1\">\n"
    "<case name=\"fakeqtest3\" exe=\"fakeqtest3\">\n"
    "<command name=\"cmd1\" />\n"
    "</case>\n"
    "</suite>\n"
    "</root>\n";

QStringList singleGreenTestTree()
{
    QStringList runnerItems;
    runnerItems
            << "0 suite1"
            << "0 0 fakeqtest3"
            << "0 0 0 cmd1"
            << "0 0 1 x"
            << "0 1 x"
            << "1 x";
    return runnerItems;
}

QMap<QString, Veritas::TestState> singleGreenTestStates()
{
    QMap<QString, Veritas::TestState> states;
    states["suite1/fakeqtest3/cmd1"] = Veritas::RunSuccess;
    return states;
}


// command
void QTestRunnerTest::singleGreenCommand()
{
    Veritas::Test* root = fetchRoot(singleGreenCommandXml);
    m_runner->setRoot(root);
    m_runner->runTests();

    m_runner->verifyTestTree(singleGreenTestTree());
    m_runner->verifyResultItems(QList<QStringList>());
    m_runner->verifyTestStates(singleGreenTestStates(), root);
    assertAllFilesClosed(root);

}

void QTestRunnerTest::nonterminatedXMLOutput()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"nonterminated\" exe=\"nonterminatedOutput\">\n"
            "<command name=\"cmd1\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    Veritas::Test* root = fetchRoot(regXML);

    m_runner->initializeGUI();
    m_runner->setRoot(root);
    m_runner->runTests(); // should not timeout
}

// command
void QTestRunnerTest::nonexistantTestExe()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"foobar\" exe=\"foobar_this_exe_does_not_exist\">\n"
            "<command name=\"cmd1\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    Veritas::Test* root = fetchRoot(regXML);

    m_runner->initializeGUI();
    m_runner->setRoot(root);
    m_runner->runTests(); // should not timeout

    QStringList result;
    result << "foobar" << i18n("Failed to start test executable.") << "/suite1/foobar_this_exe_does_not_exist" << "0";
    m_runner->verifyResultItems(QList<QStringList>() << result);
}

// command
void QTestRunnerTest::nonexistantTestCommand()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"fakeqtest1\" exe=\"fakeqtest1\">\n"
            "<command name=\"cmd_foobar_which_does_not_exist\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    Veritas::Test* root = fetchRoot(regXML);
    m_runner->setRoot(root);
    m_runner->runTests(); // should not timeout
}

// command
void QTestRunnerTest::segFault()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"segFault\" exe=\"segFault\">\n"
            "<command name=\"command\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    Veritas::Test* root = fetchRoot(regXML);
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList result;
    result << "segFault" << i18n("Test executable crashed.") << "segFault" << "0";
    m_runner->verifyResultItems(QList<QStringList>() << result);

    QMap<QString, Veritas::TestState> states;
    states["suite1/segFault"] = Veritas::RunFatal;
    states["suite1/segFault/command"] = Veritas::NoResult;
    m_runner->verifyTestStates(states, root);
}

QByteArray multipleFailuresSingleCommandXML()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"multipleFailuresSingleCommand\" exe=\"multipleFailuresSingleCommand\">\n"
            "<command name=\"command\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    return regXML;
}

QList<QStringList> multipleFailuresSingleCommandResults()
{
    QStringList result1, result2;
    KUrl dir(__FILE__);
    dir = dir.upUrl();
    result1 << "command" << "'predicate()' returned FALSE. (fail1)" << dir.pathOrUrl() + "suite1/multipleFailuresSingleCommand.cpp" << "15";
    result2 << "command" << "'predicate()' returned FALSE. (fail2)" << dir.pathOrUrl() + "suite1/multipleFailuresSingleCommand.cpp" << "18";
    return QList<QStringList>() << result1 << result2;
}

QMap<QString, Veritas::TestState> multipleFailuresSingleCommandTestStates()
{
    QMap<QString, Veritas::TestState> states;
    states["suite1/multipleFailuresSingleCommand/command"] = Veritas::RunError;
    return states;
}

// command
void QTestRunnerTest::multipleFailuresSingleCommand()
{
    QByteArray regXML = multipleFailuresSingleCommandXML();
    Veritas::Test* root = fetchRoot(regXML);
    m_runner->setRoot(root);
    m_runner->runTests();

    m_runner->verifyResultItems(multipleFailuresSingleCommandResults());
    m_runner->verifyTestStates(multipleFailuresSingleCommandTestStates(), root);
}

// command
void QTestRunnerTest::expectedFailure()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"expectedFailure\" exe=\"expectedFailure\">\n"
            "<command name=\"command\" />\n"
            "<command name=\"commandData\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    Veritas::Test* root = fetchRoot(regXML);
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList result1, result2, result3, result4;
    result1 << "command" << "expectedFailure" << "expectedFailure.cpp" << "9";
    result2 << "commandData" << "[row2] expectedFailureData1" << "expectedFailure.cpp" << "16";
    result3 << "commandData" << "[row3] Expected failure: 'pass' returned FALSE. ()" << "expectedFailure.cpp" << "19";
    result4 << "commandData" << "[row4] 'pass' returned FALSE. (fail)" << "expectedFailure.cpp" << "16";
    m_runner->verifyResultItems(QList<QStringList>() << result1 << result2 << result3 << result4);

    QMap<QString, Veritas::TestState> states;
    states["suite1/expectedFailure/command"] = Veritas::RunInfo;
    states["suite1/expectedFailure/commandData"] = Veritas::RunError;
    m_runner->verifyTestStates(states, root);
}

// command
void QTestRunnerTest::skip()
{
    QByteArray regXML =
        "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
        "<root dir=\"\">\n"
        "<suite name=\"suite1\" dir=\"suite1\">\n"
            "<case name=\"skip\" exe=\"skip\">\n"
            "<command name=\"command\" />\n"
            "<command name=\"commandData\" />\n"
            "</case>\n"
        "</suite>\n"
        "</root>\n";
    Veritas::Test* root = fetchRoot(regXML);
    m_runner->setRoot(root);
    m_runner->runTests();

    QStringList result1, result2;
    KUrl file(__FILE__);
    file = file.upUrl();
    file.addPath("suite1/skip.cpp");
    result1 << "command" << "skipCommand (skipped)" << file.toLocalFile() << "8";
    result2 << "commandData" << "[row1] skipDataCommand (skipped)" << file.toLocalFile() << "13";
    m_runner->verifyResultItems(QList<QStringList>() << result1 << result2);

    QMap<QString, Veritas::TestState> states;
    states["suite1/skip/command"] = Veritas::RunInfo;
    states["suite1/skip/commandData"] = Veritas::RunInfo;
    m_runner->verifyTestStates(states, root);
}

// helper
Veritas::Test* QTestRunnerTest::fetchRoot(QByteArray& testRegistrationXml)
{
    QBuffer buff(&testRegistrationXml);
    XmlRegister reg;
    reg.setSource(&buff);
    reg.setSettings(m_settings);
    reg.reload(0);
    return reg.root();
}

// helper
void QTestRunnerTest::assertAllFilesClosed(Veritas::Test* root)
{
    QTest::qWait(75);
    // Cases are on lvl2 in the test-tree
    for(int i=0; i<root->childCount(); i++) {
        Veritas::Test* suite = root->child(i);
        for (int i=0; i<suite->childCount(); i++) {
            Veritas::Test* test = suite->child(i);
            Case* caze = qobject_cast<Case*>(test);
            KVERIFY_MSG(caze, "Not a Case on lvl2 of the test tree? (cast failed)");
            KVERIFY_MSG(caze->executable()->fto_outputFileClosed(), caze->name());
        }
    }
}

#include "qtestrunnertest.moc"
QTEST_KDEMAIN(QTestRunnerTest, GUI)
