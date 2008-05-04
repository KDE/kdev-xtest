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

#include <resultsmodel.h>
#include <runnermodel.h>
#include <runneritem.h>

#include <QString>
#include <QStringList>

#include "kasserts.h"
#include "resultsmodeltest.h"

using QxRunner::ResultsModel;
using QxRunner::RunnerModel;
using QxRunner::RunnerItem;

namespace ResultsModelTestNm
{

class StubRunnerItem : public RunnerItem
{
public:
    StubRunnerItem(const QList<QVariant>& data, RunnerItem* parent)
            : RunnerItem(data, parent) {}

    int run() {
//         if (child(0)) {
//             setResult(QxRunner::NoResult);  // Have nothing to do as a parent
//         } else {
//             setData(1, QString::number(row()) + QString("_1"));
//             setData(2, QString::number(row()) + QString("_2"));
//             setResult(QxRunner::RunSuccess);
//         }
//         return result();
        return 0;
    }
};

class StubRunnerModel : public RunnerModel
{
public:
    StubRunnerModel()
            : RunnerModel(NULL) {
        QList<QVariant> rootData;
        rootData << tr("run_col0") << tr("run_col1") << tr("run_col2");
        setRootItem(new StubRunnerItem(rootData, NULL));

        QList<QVariant> columnData;
        columnData << "00" << "01" << "02";
        StubRunnerItem* item1 = new StubRunnerItem(columnData, rootItem());
        item1->setResult(QxRunner::RunSuccess);
        rootItem()->appendChild(item1);

        columnData.clear();
        columnData << "10" << "11" << "12";
        StubRunnerItem* item2 = new StubRunnerItem(columnData, rootItem());
        item2->setResult(QxRunner::RunFatal);
        rootItem()->appendChild(item2);
    }

    QString name() const {
        return "";
    }

};

} // end ResultsModelTestNm

using ResultsModelTestNm::StubRunnerModel;

void ResultsModelTest::init()
{
    QStringList headers;
    headers << "col0" << "col1" << "col2";
    model = new ResultsModel(headers);
    runnerModel = new StubRunnerModel();
}

void ResultsModelTest::cleanup()
{
    if (model) delete model;
    if (runnerModel) delete runnerModel;
}

// test command
void ResultsModelTest::default_()
{
    // size check
    KOMPARE(0, model->rowCount());
    KOMPARE(3, model->columnCount());

    // verify column headers
    assertColumnHeader("col0", 0);
    assertColumnHeader("col1", 1);
    assertColumnHeader("col2", 2);

    // should still be empty
    assertDataAt(QVariant(), 0, 0);
    assertDataAt(QVariant(), 0, 1);
    assertDataAt(QVariant(), 1, 1);
}

// test command
void ResultsModelTest::appendResults()
{
    fillRows();

    KOMPARE(2, model->rowCount());
    KOMPARE(QxRunner::RunSuccess, model->result(0));
    KOMPARE(QxRunner::RunFatal, model->result(1));

    checkRow(0);
    checkRow(1);
}

// test command
void ResultsModelTest::mapIndices()
{
    fillRows();
    QModelIndex runIndex0 = runnerModel->index(0,0);
    QModelIndex resultIndex0 = model->index(0,0);

    KOMPARE(runIndex0, model->mapToRunnerItemIndex(resultIndex0));
    KOMPARE(resultIndex0, model->mapFromRunnerItemIndex(runIndex0));

    KOMPARE(resultIndex0, model->mapFromRunnerItemIndex(model->mapToRunnerItemIndex(resultIndex0)));
    KOMPARE(runIndex0, model->mapToRunnerItemIndex(model->mapFromRunnerItemIndex(runIndex0)));
}

void ResultsModelTest::assertColumnHeader(const QVariant& expected, int index)
{
    KOMPARE_MSG(expected, model->headerData(index, Qt::Horizontal), "Incorrect column header caption");
}

void ResultsModelTest::assertDataAt(const QVariant& expected, int row, int column)
{
    QVariant actual = model->data(model->index(row,column), Qt::DisplayRole);
    KOMPARE_MSG(expected, actual, QString("Expected: ") + QTest::toString(expected));
}

void ResultsModelTest::checkRow(int index)
{
    QString rowStr = QString::number(index);
    assertDataAt(rowStr + "0", index, 0);
    assertDataAt(rowStr + "1", index, 1);
    assertDataAt(rowStr + "2", index, 2);
}

void ResultsModelTest::fillRows()
{
    model->addResult(runnerModel->index(0, 0)); // invoke slot
    model->addResult(runnerModel->index(1, 0)); // invoke slot
}

QTEST_MAIN( ResultsModelTest );
