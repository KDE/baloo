#include "luceneindexwriter.h"
#include <QDebug>

using namespace Baloo;

LuceneIndex::LuceneIndex(const QString& path)
{
    try {
        m_indexWriter = Lucene::newLucene<Lucene::IndexWriter>(Lucene::FSDirectory::open(path.toStdWString()),
            Lucene::newLucene<Lucene::StandardAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT),
            Lucene::IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
        m_indexReader = m_indexWriter->getReader();
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
}

void LuceneIndex::commit(bool optimize)
{
    try {
        if (optimize){
            m_indexWriter->optimize();
        }
        m_indexWriter->commit();
        m_indexReader->reopen();
    }
    catch (Lucene::LuceneException &e) {
        qWarning() << "Exception" << e.getError().c_str();
    }
}

Lucene::IndexWriterPtr LuceneIndex::indexWriter()
{
    return m_indexWriter;
}

