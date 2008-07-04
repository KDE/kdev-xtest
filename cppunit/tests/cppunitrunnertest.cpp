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

#include "cppunitrunnertest.h"

#include <kasserts.h>
#include <qtest_kde.h>
#include <ui_statuswidget.h>
#include <runnerwindow.h>
#include <runnermodel.h>
#include <testresult.h>
#include <register.h>

#include <KDebug>
#include <QBuffer>
#include <QDir>
#include <QTreeView>
#include <QModelIndex>
#include <QAbstractItemModel>

#include "testroot.h"
#include "testsuite.h"

using Veritas::RunnerWindow;
using Veritas::RunnerModel;
using Veritas::Register;
using Veritas::Test;
using CppUnit::TestSuite;
using CppUnit::TestRoot;
using CppUnit::it::CppUnitRunnerTest;

Q_DECLARE_METATYPE(QList<QStringList>)

void CppUnitRunnerTest::init()
{
    m_window = new RunnerWindow();
}

void CppUnitRunnerTest::cleanup()
{
    delete m_window;
}

// command
void CppUnitRunnerTest::empty()
{
    initNrun("./emptysuite");

    QStringList runnerItems;
    runnerItems << "0 x";
    checkTests(runnerItems);

    QList<QStringList> results;
    checkResultItems(results);

    QMap<QString, QString> status;
    status["total"] = "0";
    status["selected"] = "0";
    status["run"] = "0";
}

QStringList sunnyDayTests()
{
    QStringList runnerItems;
    runnerItems << "0 RootSuite"
                << "0 0 FooTest"
                << "0 0 0 testCmd1"
                << "0 0 1 testCmd2"
                << "0 0 2 x"
                << "0 1 BarTest"
                << "0 1 0 testCmd1"
                << "0 1 1 testCmd2"
                << "0 1 2 x"
                << "0 2 BazTest"
                << "0 2 0 testCmd1"
                << "0 2 1 x"
                << "0 3 x"
                << "1 x";
    return runnerItems;
}

QMap<QString,QString> sunnyDayStatus()
{
    QMap<QString,QString> status;
    status["total"] = "5";
    status["selected"] = "5";
    status["run"] = "5";
    status["success"] = ": 4";
    status["errors"] = ": 1";
    status["warnings"] = ": 0";
    return status;
}

// command
void CppUnitRunnerTest::sunnyDay()
{
    initNrun("./sunnysuite");
    checkTests(sunnyDayTests());

    QStringList result0;
    result0 << "testCmd2" << "" << QDir::currentPath() + "/fake_sunnysuite.cpp" << "63";
    QList<QStringList> results;
    results << result0;
    checkResultItems(results);

    checkStatusWidget(sunnyDayStatus());
}

// command
void CppUnitRunnerTest::runTwice()
{
    KTODO;
}

// command
void CppUnitRunnerTest::multiSuite()
{
    initNrun("./multisuite");

    QStringList topo;
    topo << "0 FooSuite"
         << "0 0 FooTest"
         << "0 0 0 FooTest::fooCmd"
         << "0 0 1 x"
         << "0 1 x"
         << "1 BarSuite"
         << "1 0 BarTest"
         << "1 0 0 BarTest::barCmd"
         << "1 0 1 x"
         << "1 1 x"
         << "2 x";
    checkTests(topo);

    QList<QStringList> results;
    checkResultItems(results);

    QMap<QString,QString> status;
    status["total"] = "2";
    status["selected"] = "2";
    status["run"] = "2";
    status["success"] = ": 2";
    status["errors"] = ": 0";
    status["warnings"] = ": 0";
    checkStatusWidget(status);

}

// helper
void CppUnitRunnerTest::checkResultItems(QList<QStringList> expected)
{
    nrofMessagesEquals(expected.size());
    for (int i = 0; i < expected.size(); i++)
        checkResultItem(i, expected.value(i));
}

// helper
void CppUnitRunnerTest::checkResultItem(int num, const QStringList& item)
{
    QAbstractItemModel* results = m_window->ui().treeResults->model();
    KOMPARE(item[0], results->data(results->index(num, 0))); // test name
    //KOMPARE(item[1], results->data(results->index(num, 1))); // status -> "Error"
    // commented out since this row is currently hidden
    KOMPARE(item[1], results->data(results->index(num, 2))); // failure message
    // KOMPARE(item[2], results->data(results->index(num, 3))); // filename
    // commented out since this is factually a file in the source-tree, for
    // which i can not get the proper location at runtime.
    KOMPARE(item[3], results->data(results->index(num, 4))); // line number
}

// helper
void CppUnitRunnerTest::nrofMessagesEquals(int num)
{
    QAbstractItemModel* results = m_window->ui().treeResults->model();
    for (int i = 0; i < num; i++) {
        KVERIFY_MSG(results->index(i, 0).isValid(),
                    QString("Expected ") + QString::number(num) + " items but got " + QString::number(i));
    }
    KVERIFY(!results->index(num, 0).isValid());
}

// helper
void CppUnitRunnerTest::initNrun(const char* exe)
{
    Register<TestRoot, TestSuite> reg;
    reg.addFromExe(QFileInfo(exe));
    reg.rootItem()->setExecutable(QFileInfo(exe));
    RunnerModel* model = new RunnerModel;
    model->setRootItem(reg.rootItem());
    model->setExpectedResults(Veritas::RunError);
    m_window = new RunnerWindow;
    m_window->setModel(model);
    m_window->show();
    m_window->ui().actionStart->trigger();
    if (!QTest::kWaitForSignal(m_window->runnerModel(), SIGNAL(allItemsCompleted()), 2000))
//    if (!QTest::kWaitForSignal(win->runnerModel(), SIGNAL(allItemsCompleted())))
       QFAIL("Timeout while waiting for runner items to complete execution");
}

// helper
void CppUnitRunnerTest::checkStatusWidget(QMap<QString, QString> labels)
{
    Ui::StatusWidget* status = m_window->statusWidget();

    // validate the status widget
    KOMPARE(labels["total"], status->labelNumTotal->text());
    KOMPARE(labels["selected"], status->labelNumSelected->text());
    KOMPARE(labels["run"], status->labelNumRun->text());

    if (labels.contains("success"))
        KOMPARE(labels["success"], status->labelNumSuccess->text());
    if (labels.contains("errors"))
        KOMPARE(labels["errors"], status->labelNumErrors->text());
    if (labels.contains("warnings"))
        KOMPARE(labels["warnings"], status->labelNumWarnings->text());
}

// helper
void CppUnitRunnerTest::checkTests(QStringList runnerItems)
{
    foreach(QString s, runnerItems) {
        QStringList spl = s.split(" ");
        int lvl0 = spl[0].toInt();
        int lvl1 = (spl.size() > 2) ? spl[1].toInt() : -1;
        int lvl2 = (spl.size() > 3) ? spl[2].toInt() : -1;
        QVariant exp = (spl.last() == "x") ? QVariant() : spl.last();
        checkTest(exp, lvl0, lvl1, lvl2);
    }
}

// helper
void CppUnitRunnerTest::checkTest(const QVariant& expected, int lvl0, int lvl1, int lvl2)
{
    QAbstractItemModel* runner = m_window->ui().treeRunner->model();
    QModelIndex index = runner->index(lvl0, 0);
    if (lvl1 != -1) {
        index = runner->index(lvl1, 0, index);
        if (lvl2 != -1) {
            index = runner->index(lvl2, 0, index);
        }
    }
    KOMPARE(expected, runner->data(index));
}

#include "cppunitrunnertest.moc"
QTEST_KDEMAIN(CppUnitRunnerTest, GUI)
