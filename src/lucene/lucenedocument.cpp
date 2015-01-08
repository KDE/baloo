#include "lucenedocument.h"

using namespace Baloo;


LuceneDocument::LuceneDocument()
{
}

LuceneDocument::LuceneDocument(const Lucene::DocumentPtr& doc)
    : m_doc(doc)
{
}

void LuceneDocument::addField(const QString& field, const QString& value, Lucene::Field::Store store, 
                                       Lucene::Field::Index index)
{
    m_doc->add(Lucene::newLucene<Lucene::Field>(field.toStdWString(), value.toStdWString(), store, index));
}

void LuceneDocument::addField(const QString& field, const int value, Lucene::Field::Store store, 
                              Lucene::Field::Index index)
{
    addField(field, QString::number(value), store, index);
}

Lucene::DocumentPtr LuceneDocument::doc() const
{
    return m_doc;
}


