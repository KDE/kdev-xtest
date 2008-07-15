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

#include "qtestoutputparser.h"
#include "qtestcase.h"

#include <kdevplatform/veritas/test.h>

#include <QStringRef>
#include <KDebug>

using QTest::QTestCase;
using QTest::QTestOutputParser;
using Veritas::Test;
using Veritas::TestResult;

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


const QString QTestOutputParser::c_testfunction("TestFunction");
const QString QTestOutputParser::c_description("Description");
const QString QTestOutputParser::c_incident("Incident");
const QString QTestOutputParser::c_type("type");
const QString QTestOutputParser::c_file("file");
const QString QTestOutputParser::c_line("line");
const QString QTestOutputParser::c_pass("pass");
const QString QTestOutputParser::c_message("Message");
const QString QTestOutputParser::c_fail("fail");
const QString QTestOutputParser::c_initTestCase("initTestCase");
const QString QTestOutputParser::c_cleanupTestCase("cleanupTestCase");

QTestOutputParser::QTestOutputParser(QIODevice* device)
    : QXmlStreamReader(device), m_result(0),
      m_processingTestFunction(false),
      m_fillingResult(false),
      m_settingFailure(false)
{
}

QTestOutputParser::~QTestOutputParser()
{
}

bool QTestOutputParser::isStartElement_(const QString& elementName)
{
    return isStartElement() && (name() == elementName);
}

bool QTestOutputParser::isEndElement_(const QString& elementName)
{
    return isEndElement() && (name() == elementName);
}

void QTestOutputParser::go(QTestCase* caze)
{
    if (!device()->isOpen())
        device()->open(QIODevice::ReadOnly);
    if (!device()->isReadable()) {
        // do something
    }
    m_case = caze;

    if (m_settingFailure)
        setFailure();
    if (m_processingTestFunction)
        processTestFunction();

    while (!atEnd() && doingOK()) {
        readNext();
        if (isStartElement_(c_testfunction)) {
            processTestFunction();
        }
    }
    kError(hasError()) << errorString() << " @ " << lineNumber() << ":" << columnNumber();
}

bool QTestOutputParser::doingOK() const
{
    return (!m_result || m_result->state() == Veritas::NoResult 
                      || m_result->state() == Veritas::RunSuccess);
}

bool QTestOutputParser::fixtureFailed(const QString& cmd)
{
    if (cmd != c_initTestCase && cmd != c_cleanupTestCase)
        return false;
    return !doingOK();
}

void QTestOutputParser::processTestFunction()
{
    QString cmdName;
    if (!m_processingTestFunction) {
        cmdName = attributes().value("name").toString();
        Test* cmd = m_case->childNamed(cmdName);
        m_result = new TestResult; // this probably leaks. TODO
        if (cmd)
            cmd->started();
    }
    m_processingTestFunction = true;
    while (!atEnd() && !isEndElement_(c_testfunction)) {
        readNext();
        if (isStartElement_(c_incident))
            fillResult();
        if (isStartElement_(c_message))
            appendMsg();
    }
    if (isEndElement_(c_testfunction)) {
        Test* cmd = m_case->childNamed(cmdName);
        if (cmd) {
            cmd->setResult(m_result);
            cmd->finished();
        } else if (fixtureFailed(cmdName)) {
            kDebug() << "init/cleanup TestCase failed";
            m_case->started();
            m_case->setResult(m_result);
            m_case->finished();
        }
        m_processingTestFunction = false;
    }
}

void QTestOutputParser::fillResult()
{
    QString type = attributes().value(c_type).toString();
    if (type == c_pass)
        setSuccess();
    else if (type == c_fail)
        setFailure();
}

void QTestOutputParser::appendMsg()
{
    while (!atEnd() && !isEndElement_(c_message)) {
        readNext();
/*        if (isStartElement_(c_description)) {
            m_result->addOutputLine(readElementText().toAscii());
        }*/
    }
}

void QTestOutputParser::setSuccess()
{
    m_result->setState(Veritas::RunSuccess);
}

void QTestOutputParser::setFailure()
{
    if (!m_settingFailure) {
        m_result->setState(Veritas::RunError);
        m_result->setFile(QFileInfo(attributes().value(c_file).toString()));
        m_result->setLine(attributes().value(c_line).toString().toInt());
    }
    m_settingFailure = true;

    while (!atEnd() && !isEndElement_(c_description)) {
        readNext();
        if (isCDATA())
            m_result->setMessage(text().toString());
    }

    if (isEndElement_(c_description))
        m_settingFailure = false;
}
