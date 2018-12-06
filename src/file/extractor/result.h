/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EXTRACTIONRESULT_H
#define EXTRACTIONRESULT_H

#include <KFileMetaData/ExtractionResult>

#include "document.h"
#include "termgenerator.h"

/**
 * \class Result result.h
 *
 * \brief The result class is where all the data extracted by
 * the KFileMetaData extractors is saved to.
 * It implements the KFileMetaData::ExtractionResult interface.
 * The results can be retrieved as a Baloo::Document using
 * \c document() and stored in the database.
 *
 */
class Result : public KFileMetaData::ExtractionResult
{
public:
    Result();
    Result(const QString& url, const QString& mimetype, const Flags& flags = ExtractEverything);

    void add(KFileMetaData::Property::Property property, const QVariant& value) override;
    void append(const QString& text) override;
    void addType(KFileMetaData::Type::Type type) override;

    /**
     * Can be used to add extraction results to an existing
     * Baloo::Document. Has to be called before passing the
     * Result to KFileMetaData::Extractor::extract().
     * \param doc The Baloo::Document
     */
    void setDocument(const Baloo::Document& doc);

    /**
     * Returns the Baloo document to which the results from the extractors
     * are saved.
     */
    Baloo::Document& document() {
        return m_doc;
    }

    /**
     * Applies the finishing touches on the document, and makes
     * it ready to be pushed into the db.
     */
    void finish();

private:
    /**
     * The document that represents the file that is to be indexed.
     */
    Baloo::Document m_doc;
    /**
     * Contains the position state of the metadata and adds it to the document.
     */
    Baloo::TermGenerator m_termGen;
    /**
     * Contains the position state of plain text and adds it to the document.
     */
    Baloo::TermGenerator m_termGenForText;
    /**
     * Contains all indexed property data from the extractors.
     */
    QVariantMap m_map;
};

#endif // EXTRACTIONRESULT_H
