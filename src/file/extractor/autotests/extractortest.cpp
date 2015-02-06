/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "extractortest.h"
#include "config.h"

#include <QTest>
#include <QProcess>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

#include "xapiandatabase.h"
#include "xapiandocument.h"
#include "luceneindex.h"
#include "lucenedocument.h"
#include <lucene++/TermAttribute.h>

void ExtractorTest::test()
{
    QTemporaryDir dbDir;
    const QString fileUrl = dbDir.path() + QLatin1String("/testFile.txt");

    QFile file(fileUrl);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << "Some words";
    file.close();

    QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor"));

    QStringList args;
    args << fileUrl << QLatin1String("--db") << dbDir.path() << QLatin1String("--ignoreConfig");

    QProcess process;
    process.start(exe, args);
    process.setProcessChannelMode(QProcess::MergedChannels);
    QVERIFY(process.waitForFinished(10000));

    Baloo::LuceneIndex index(dbDir.path());
    Lucene::IndexReaderPtr reader = index.IndexReader();

    QCOMPARE((int)reader->numDocs(), 1);
    Baloo::LuceneDocument doc(reader->document(0));
    Lucene::Collection<Lucene::FieldablePtr> fields = doc.doc()->getFields();
    QStringList words;
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        Lucene::FieldablePtr field = *it;

        //FIXME commented out code below causes a segfault on line tokenStream->incrementToken() lucene++ bug??
        //This prevents us from viewing how the terms are tokenized

        /*if (field->isTokenized()) {
            Lucene::AnalyzerPtr analyzer = index.indexWriter()->getAnalyzer();
            Lucene::TokenStreamPtr tokenStream = analyzer->tokenStream(L"", field->readerValue());
            Lucene::TermAttributePtr termAtt = tokenStream->getAttribute<Lucene::TermAttribute>();
            tokenStream->reset();
            while (tokenStream->incrementToken()) {
                //Lucene::String term = termAtt->term();
            }
        }*/

        QString str = QString::fromStdWString(field->stringValue());
        qDebug() << QString::fromStdWString(field->name()) << str;
        words << str;
    }

    QVERIFY(words.contains(QLatin1String("Some words")));
    QVERIFY(words.contains(QLatin1String("testFile.txt")));
    QVERIFY(words.contains(QLatin1String("2")));
}

void ExtractorTest::testFileDeletion()
{
    QTemporaryDir dbDir;
    Baloo::LuceneIndex index(dbDir.path());
    Baloo::LuceneDocument doc;
    doc.addIndexedField(QStringLiteral("foo"), QStringLiteral("bar"));
    QString randomUrl = dbDir.path() + "/" "foo";
    doc.addIndexedField(QStringLiteral("URL"), randomUrl);
    index.addDocument(doc);
    index.commit();
    Lucene::IndexReaderPtr reader = index.IndexReader();
    QCOMPARE((int)reader->numDocs(), 1);
    QCOMPARE((int)reader->document(0)->getFields().size(), 2);

    QString exe = QStandardPaths::findExecutable(QLatin1String("baloo_file_extractor"));
    
    QStringList args;
    args << randomUrl << QLatin1String("--db") << dbDir.path() << QLatin1String("--ignoreConfig");
    QProcess process;
    process.start(exe, args);
    QVERIFY(process.waitForFinished(10000));
    // The document should have been deleted from the db

    reader = index.IndexReader();
    QCOMPARE((int)reader->numDocs(), 0);
}


QTEST_MAIN(ExtractorTest)
