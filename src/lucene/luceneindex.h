#ifndef BALOO_LUCENEINDEXWRITER_H
#define BALOO_LUCENEINDEXWRITER_H

#include <lucene++/LuceneHeaders.h>
#include "lucenedocument.h"
#include "lucene_export.h"

namespace Baloo {
class BALOO_LUCENE_EXPORT LuceneIndex {
public:
    LuceneIndex(const QString& path);
    ~LuceneIndex();

    void addDocument(LuceneDocument& doc);
    void addDocument(Lucene::DocumentPtr doc);
    void commit(bool optimize = 0);
    Lucene::IndexWriterPtr indexWriter() { return m_indexWriter; }
    Lucene::IndexReaderPtr IndexReader();

private:
    Lucene::IndexWriterPtr m_indexWriter;
};
}

#endif // BALOO_LUCENEINDEXWRITER_H