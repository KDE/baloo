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

void LuceneDocument::addBoolTerm(const QString& term, const QString& prefix)
{
  addField(prefix, term, Lucene::Field::STORE_NO, Lucene::Field::INDEX_NOT_ANALYZED);
}

void LuceneDocument::indexText(const QString& term, const QString& prefix)
{
    addField(prefix, term, Lucene::AbstractField::STORE_YES, Lucene::AbstractField::INDEX_ANALYZED);
}



Lucene::DocumentPtr LuceneDocument::doc() const
{
    return m_doc;
}


