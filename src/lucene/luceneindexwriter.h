#ifndef BALOO_LUCENEINDEXWRITER_H
#define BALOO_LUCENEINDEXWRITER_H

#include <LuceneHeaders.h>
#include "lucenedocument.h"

namespace Baloo {
class LuceneIndexWriter {
    LuceneIndexWriter(const QString& path);
    ~LuceneIndexWriter();
    
    void addDocument(Lucene::DocumentPtr& doc);
    void addDocument(LuceneDocument& doc);
    
private:
    Lucene::IndexWriterPtr m_indexWriter;
};
}

#endif // BALOO_LUCENEINDEXWRITER_H