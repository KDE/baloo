#include "luceneindex.h"
#include <QDebug>
#include <QString>

using namespace Baloo;

LuceneIndex::LuceneIndex(const QString& path, bool writeOnly)
    : m_path(path)
    , m_writeOnly(writeOnly)
{
    if (m_writeOnly) {
        m_indexWriter = openWriter();
    }
    else {
        //creates index if it doesn't already exist
        openWriter();

        m_indexReader = Lucene::IndexReader::open(Lucene::FSDirectory::open( (m_path.toStdWString()) ), true);
    }
}

LuceneIndex::~LuceneIndex()
{
}


void LuceneIndex::addDocument(LuceneDocument& doc)
{
    addDocument(doc.doc());
}

void LuceneIndex::addDocument(Lucene::DocumentPtr doc)
{
    if (m_writeOnly) {
        try {
            m_indexWriter->addDocument(doc);
        }
        catch (Lucene::LuceneException &e) {
            qWarning() << "Exception" << e.getError().c_str();
        }
    }
    else {
        m_docsToAdd << doc;
    }
}

void LuceneIndex::replaceDocument(const QString& url, const Lucene::DocumentPtr& doc)
{
    if (m_writeOnly) {
        m_indexWriter->updateDocument(makeTerm(QStringLiteral("URL"), url), doc);
    }
    else {
        m_docsToReplace << qMakePair(url, doc);
    }
}

void LuceneIndex::deleteDocument(const QString& url)
{
    if (m_writeOnly) {
        m_indexWriter->deleteDocuments( makeTerm(QStringLiteral("URL"), url) );
    }
    else {
        m_docsToDelete << url;
    }
}

Lucene::TermPtr LuceneIndex::makeTerm(const QString& field, const QString& value)
{
    Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(field.toStdWString(), value.toStdWString());
    return term;
}

Lucene::IndexReaderPtr LuceneIndex::IndexReader()
{
    if (m_indexReader) {
        reopenReader();
        return m_indexReader;
    }
    else {
        return m_indexWriter->getReader();
    }
}

Lucene::IndexWriterPtr LuceneIndex::openWriter()
{
    Lucene::IndexWriterPtr writer = Lucene::newLucene<Lucene::IndexWriter>(Lucene::FSDirectory::open( (m_path.toStdWString()) ),
                                        Lucene::newLucene<Lucene::StandardAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT),
                                        Lucene::IndexWriter::MaxFieldLengthLIMITED);
    return writer;
}

bool LuceneIndex::haveChanges()
{
    return !m_docsToAdd.isEmpty() || !m_docsToReplace.isEmpty() || !m_docsToDelete.isEmpty();
}

void LuceneIndex::commit()
{
    if (m_writeOnly) {
        try {
            m_indexWriter->commit();
        }
        catch (Lucene::LuceneException &e) {
            qWarning() << "Exception" << e.getError().c_str();
        }
    }

    if (!haveChanges()) {
        return;
    }

    Lucene::IndexWriterPtr writer = openWriter();
    Q_FOREACH (const Lucene::DocumentPtr& doc, m_docsToAdd) {
        writer->addDocument(doc);
    }

    Q_FOREACH (const urlDocPair& pair, m_docsToReplace) {
        writer->updateDocument( makeTerm(QStringLiteral("URL"), pair.first) , pair.second );
    }

    Q_FOREACH (const QString url, m_docsToDelete) {
        writer->deleteDocuments( makeTerm(QStringLiteral("URL"), url) );
    }

    writer->commit();
    reopenReader();
    m_docsToAdd.clear();
    m_docsToDelete.clear();
    m_docsToReplace.clear();
}

void LuceneIndex::reopenReader()
{
    Lucene::IndexReaderPtr newReader = m_indexReader->reopen();
    if (newReader != m_indexReader) {
        m_indexReader->close();
        m_indexReader = newReader;
    }
}
