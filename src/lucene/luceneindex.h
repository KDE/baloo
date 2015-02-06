#ifndef BALOO_LUCENEINDEXWRITER_H
#define BALOO_LUCENEINDEXWRITER_H

#include <lucene++/LuceneHeaders.h>
#include "lucenedocument.h"
#include "lucene_export.h"
#include <QVector>
#include <QPair>
#include <QStringList>

namespace Baloo {
class BALOO_LUCENE_EXPORT LuceneIndex {
public:
    LuceneIndex(const QString& path, bool writeOnly = false);
    ~LuceneIndex();

    void addDocument(LuceneDocument& doc);
    void addDocument(Lucene::DocumentPtr doc);
    void replaceDocument(const QString& url, const Lucene::DocumentPtr& doc);
    void deleteDocument(const QString& url);
    void commit();
    bool haveChanges();
    Lucene::IndexWriterPtr indexWriter() { return m_indexWriter; }
    QString path() { return m_path; }
    Lucene::IndexReaderPtr IndexReader();

private:
    Lucene::TermPtr makeTerm(const QString& field, const QString& value);
    Lucene::IndexWriterPtr openWriter();
    void reopenReader();

    Lucene::IndexWriterPtr m_indexWriter;
    Lucene::IndexReaderPtr m_indexReader;
    QString m_path;
    bool m_writeOnly;
    QVector<Lucene::DocumentPtr> m_docsToAdd;
    typedef QPair<QString, Lucene::DocumentPtr> urlDocPair;
    QVector<urlDocPair> m_docsToReplace;
    QStringList m_docsToDelete;
};
}

#endif // BALOO_LUCENEINDEXWRITER_H