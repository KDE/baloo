/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014 Laurent Montel <montel@kde.org>
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

#include "akonotesindexer.h"

#include <QTextDocument>


AkonotesIndexer::AkonotesIndexer(const QString& path)
    : AbstractIndexer(), m_termGen( 0 )
{
    m_db = new Xapian::WritableDatabase(path.toUtf8().constData(), Xapian::DB_CREATE_OR_OPEN);
}

AkonotesIndexer::~AkonotesIndexer()
{
    m_db->commit();
    delete m_db;
}

QStringList AkonotesIndexer::mimeTypes() const
{
    return QStringList() << QString::fromLatin1( "text/x-vnd.akonadi.note" );
}

void AkonotesIndexer::index(const Akonadi::Item &item)
{
    KMime::Message::Ptr msg;
    try {
        msg = item.payload<KMime::Message::Ptr>();
    } catch (const Akonadi::PayloadException&) {
        return;
    }
    m_doc = new Xapian::Document();
    m_termGen = new Xapian::TermGenerator();
    m_termGen->set_document(*m_doc);
    m_termGen->set_database(*m_db);

    process(msg);

    Akonadi::Entity::Id colId = item.parentCollection().id();
    QByteArray term = 'C' + QByteArray::number(colId);
    m_doc->add_boolean_term(term.data());

    m_db->replace_document(item.id(), *m_doc);

    delete m_doc;
    delete m_termGen;

    m_doc = 0;
    m_termGen = 0;
}

void AkonotesIndexer::process(const KMime::Message::Ptr &msg)
{
    //
    // Process Headers
    // (Give the subject a higher priority)
    KMime::Headers::Subject* subject = msg->subject(false);
    if (subject) {
        std::string str(subject->asUnicodeString().toUtf8().constData());
        kDebug() << "Indexing" << str.c_str();
        m_termGen->index_text_without_positions(str, 1, "SU");
        m_termGen->index_text_without_positions(str, 100);
        m_doc->set_data(str);
    }

    KMime::Content* mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
       const std::string text(mainBody->decodedText().toUtf8().constData());
       m_termGen->index_text_without_positions(text);
       m_termGen->index_text_without_positions(text, 1, "BO");
    } else {
        processPart(msg.get(), 0);
    }
}

void AkonotesIndexer::processPart(KMime::Content* content, KMime::Content* mainContent)
{
    if (content == mainContent) {
        return;
    }

    KMime::Headers::ContentType* type = content->contentType(false);
    if (type) {
        if (type->isMultipart()) {
            if (type->isSubtype("encrypted"))
                return;

            Q_FOREACH (KMime::Content* c, content->contents()) {
                processPart(c, mainContent);
            }
        }

        // Only get HTML content, if no plain text content
        if (!mainContent && type->isHTMLText()) {
            QTextDocument doc;
            doc.setHtml(content->decodedText());

            const std::string text(doc.toPlainText().toUtf8().constData());
            m_termGen->index_text_without_positions(text);
        }
    }
}


void AkonotesIndexer::commit()
{
    m_db->commit();
}

void AkonotesIndexer::remove(const Akonadi::Item &item)
{
    try {
        m_db->delete_document(item.id());
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void AkonotesIndexer::remove(const Akonadi::Collection& collection)
{
    try {
        Xapian::Query query('C'+ QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*m_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->get_doccount());
        Xapian::MSetIterator end(mset.end());
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void AkonotesIndexer::move(const Akonadi::Item::Id& itemId,
                        const Akonadi::Entity::Id& from,
                        const Akonadi::Entity::Id& to)
{
    Xapian::Document doc;
    try {
        doc = m_db->get_document(itemId);
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }

    const QByteArray ft = 'C' + QByteArray::number(from);
    const QByteArray tt = 'C' + QByteArray::number(to);

    doc.remove_term(ft.data());
    doc.add_boolean_term(tt.data());
    m_db->replace_document(doc.get_docid(), doc);
}

