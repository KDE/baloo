#include "luceneindexwriter.h"
#include <QDebug>

using namespace Baloo;

LuceneIndexWriter::LuceneIndexWriter(const QString& path)
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

void LuceneIndexWriter::addDocument(LuceneDocument& doc)
{
    addDocument(doc.doc());
}

void LuceneIndexWriter::addDocument(Lucene::DocumentPtr doc)
{
    try {
        m_indexWriter->addDocument(doc);
    }
    catch (Lucene::LuceneException &e) {
        qWarning() << "Exception" << e.getError().c_str();
    }
}
