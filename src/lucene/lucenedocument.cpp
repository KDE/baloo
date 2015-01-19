#include "lucenedocument.h"
#include <QStringList>

using namespace Baloo;


LuceneDocument::LuceneDocument()
{
    m_doc = Lucene::newLucene<Lucene::Document>();
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

void LuceneDocument::addIndexedField(const QString& field, const QString& value, bool stored)
{
    if (stored) {
      addField(field, value, Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED);
    }
    else {
      addField(field, value, Lucene::Field::STORE_NO, Lucene::Field::INDEX_NOT_ANALYZED);
    }
}


void LuceneDocument::addNumericField(const QString& name, long int value, bool storeLong)
{
    Lucene::NumericFieldPtr numericField = Lucene::newLucene<Lucene::NumericField>(name.toStdWString());
    if (storeLong) {
	numericField->setLongValue(value);
    }
    else {
	numericField->setIntValue(value);
    }
    m_doc->add(numericField);
}

void LuceneDocument::indexText(const QString& term, const QString& prefix)
{
    addField(prefix, term, Lucene::AbstractField::STORE_YES, Lucene::AbstractField::INDEX_ANALYZED);
    //TODO check what sort of analyzer lucene uses by default
}

QStringList LuceneDocument::getFieldValues(const QString& field)
{
    Lucene::Collection<Lucene::String> values = m_doc->getValues(field.toStdWString());
    QStringList vals;
    for (Lucene::Collection<Lucene::String>::iterator it = values.begin(); it != values.end(); ++it) {
        vals << QString::fromStdWString(*it);
    }
    return vals;
}

bool LuceneDocument::removeFields(QString& field)
{
    bool modified = true;
    if (!m_doc->getField(field.toStdWString())) {
        modified = false;
    }
    else {
        m_doc->removeFields(field.toStdWString());
    }
    return modified;
}

Lucene::DocumentPtr LuceneDocument::doc() const
{
    return m_doc;
}


