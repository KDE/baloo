#ifndef BALOO_LUCENEDOCUMENT_H
#define BALOO_LUCENEDOCUMENT_H

#include <LuceneHeaders.h>
#include <QString>

namespace Baloo {

/**
 * This class is a wraper over a lucene doc for providing Qt APIs
 */
class LuceneDocument
{
    LuceneDocument();
    LuceneDocument(const Lucene::DocumentPtr& doc);

    void addField(const QString &field, const QString &value, Lucene::Field::Store store,
                  Lucene::Field::Index index);
    void addField(const QString& field, const int value, Lucene::Field::Store store, 
                                  Lucene::Field::Index index);

    Lucene::DocumentPtr doc() const;


private:
    Lucene::DocumentPtr m_doc;
};
}

#endif //BALOO_LUCENEDOCUMENT_H