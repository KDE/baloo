#include "luceneindex.h"
#include <QDebug>
#include <QString>

using namespace Baloo;

LuceneIndex::LuceneIndex(const QString& path)
    : m_path(path)
{
    try {
        m_indexWriter = Lucene::newLucene<Lucene::IndexWriter>(Lucene::FSDirectory::open(path.toStdWString()),
            Lucene::newLucene<Lucene::StandardAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT),
            Lucene::IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
    }
    catch (Lucene::LuceneException& e) {
         qWarning() << "Exception:" << e.getError().c_str();
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
    try {
        m_indexWriter->addDocument(doc);
    }
    catch (Lucene::LuceneException &e) {
        qWarning() << "Exception" << e.getError().c_str();
    }
    m_haveChanges = true;
}

void LuceneIndex::replaceDocument(const QString& url, const Lucene::DocumentPtr& doc)
{
    m_indexWriter->updateDocument(makeTerm(QStringLiteral("URL"), url), doc);
    m_haveChanges = true;
}

void LuceneIndex::deleteDocument(const QString& url)
{
    m_indexWriter->deleteDocuments( makeTerm(QStringLiteral("URL"), url) );
}

Lucene::TermPtr LuceneIndex::makeTerm(const QString& field, const QString& value)
{
    Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(field.toStdWString(), value.toStdWString());
    return term;
}

Lucene::IndexReaderPtr LuceneIndex::IndexReader()
{
    return m_indexWriter->getReader();
}

void LuceneIndex::commit(bool optimize)
{
    try {
        if (optimize){
            m_indexWriter->optimize();
        }
        m_indexWriter->commit();
    }
    catch (Lucene::LuceneException &e) {
        qWarning() << "Exception" << e.getError().c_str();
    }
    m_haveChanges = false;
}


