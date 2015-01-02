#ifndef BALOO_LUCENEIDEXWRITER_H
#define BALOO_LUCENEINDEXWRITER_H

#include <lucene++/LuceneHeaders.h>
#include "lucenedocument.h"

namespace Baloo {
class LuceneIndexWriter {
    LuceneIndexWriter(const QString& path);
    ~LuceneIndexWriter();
    
    void addDocument(const Lucene::DocumentPtr& doc);
    void addDocument(const LuceneDocument& doc);
    
private:
    Lucene::IndexWriterPtr m_indexWriter;
};
}