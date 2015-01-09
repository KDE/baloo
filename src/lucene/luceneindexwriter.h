#ifndef BALOO_LUCENEINDEXWRITER_H
#define BALOO_LUCENEINDEXWRITER_H

#include <lucene++/LuceneHeaders.h>
#include "lucenedocument.h"

namespace Baloo {
class LuceneIndexWriter {
public:
    LuceneIndexWriter(const QString& path);
    ~LuceneIndexWriter();
    
    void addDocument(LuceneDocument& doc);
    void addDocument(Lucene::DocumentPtr doc);
    
private:
    Lucene::IndexWriterPtr m_indexWriter;
};
}

#endif // BALOO_LUCENEINDEXWRITER_H