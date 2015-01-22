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
    void replaceDocument(const QString& url, const Lucene::DocumentPtr& doc);
    void deleteDocument(const QString& url);
    void commit(bool optimize = 0);
    bool haveChanges() { return m_haveChanges; }
    Lucene::TermPtr makeTerm(const QString& field, const QString& value);
    Lucene::IndexWriterPtr indexWriter() { return m_indexWriter; }
    Lucene::IndexReaderPtr IndexReader();

private:
    Lucene::IndexWriterPtr m_indexWriter;
    bool m_haveChanges = false;
};
}

#endif // BALOO_LUCENEINDEXWRITER_H