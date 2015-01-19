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
    Lucene::IndexWriterPtr indexWriter();

private:
    Lucene::IndexWriterPtr m_indexWriter;
    Lucene::IndexReaderPtr m_indexReader;
};
}

#endif // BALOO_LUCENEINDEXWRITER_H