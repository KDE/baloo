#ifndef BALOO_LUCENEDOCUMENT_H
#define BALOO_LUCENEDOCUMENT_H

#include <lucene++/LuceneHeaders.h>
#include <QString>
#include "lucene_export.h"

namespace Baloo {

/**
 * This class is a wraper over a lucene doc for providing Qt APIs
 */
class BALOO_LUCENE_EXPORT LuceneDocument
{
public:
    LuceneDocument();
    LuceneDocument(const Lucene::DocumentPtr& doc);
    void addIndexedField(const QString& field, const QString& value, bool stored = 1);
    void addNumericField( const QString& name, long int value, bool storeLong = 0);
    void indexText(const QString& term, const QString& prefix = QStringLiteral("content"));
    Lucene::DocumentPtr doc() const;
    QStringList getFieldValues(const QString& field);


private:
    Lucene::DocumentPtr m_doc;
    void addField(const QString &field, const QString &value, Lucene::Field::Store store,
                  Lucene::Field::Index index);
    void addField(const QString& field, int value, Lucene::Field::Store store,
                                  Lucene::Field::Index index);
};
}

#endif //BALOO_LUCENEDOCUMENT_H
