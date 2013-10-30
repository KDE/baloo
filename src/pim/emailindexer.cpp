/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "emailindexer.h"

#include <Akonadi/Collection>

#include <QTextDocument>

EmailIndexer::EmailIndexer(const QString& path)
{
    m_db = new Xapian::WritableDatabase(path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
}

EmailIndexer::~EmailIndexer()
{
    m_db->commit();
    delete m_db;
}

void EmailIndexer::index(const Akonadi::Item& item)
{
    if (!item.hasPayload()) {
        kDebug() << "No payload";
        return;
    }

    if (!item.hasPayload<KMime::Message::Ptr>()) {
        return;
    }

    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    if (status.isSpam())
        return;

    m_doc = new Xapian::Document();
    m_termGen = new Xapian::TermGenerator();
    m_termGen->set_document(*m_doc);
    m_termGen->set_database(*m_db);

    processMessageStatus(status);

    KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
    process(msg);

    // Parent collection
    Akonadi::Entity::Id colId = item.parentCollection().id();
    QByteArray term = 'C' + QByteArray::number(colId);
    m_doc->add_boolean_term(term.data());

    m_db->replace_document(item.id(), *m_doc);

    delete m_doc;
    delete m_termGen;

    m_doc = 0;
    m_termGen = 0;
}

void EmailIndexer::insert(const QByteArray& key, KMime::Headers::Generics::MailboxList* mlist)
{
    if (mlist)
        insert(key, mlist->mailboxes());
}

void EmailIndexer::insert(const QByteArray& key, KMime::Headers::Generics::AddressList* alist)
{
    if (alist)
        insert(key, alist->mailboxes());
}

// Add once with a prefix and once without
void EmailIndexer::insert(const QByteArray& key, const KMime::Types::Mailbox::List& list)
{
    Q_FOREACH (const KMime::Types::Mailbox& mbox, list) {
        std::string name = mbox.name().toStdString();
        m_termGen->index_text_without_positions(name, 1, key.data());
        m_termGen->index_text_without_positions(name, 1);

        m_doc->add_term((key + mbox.address()).data());
        m_doc->add_term(mbox.address().data());
    }
}

// FIXME: Only index properties that are actually searched!
void EmailIndexer::process(const KMime::Message::Ptr& msg)
{
    //
    // Process Headers
    //
    KMime::Headers::Subject* subject = msg->subject(false);
    if (subject) {
        QString str = subject->asUnicodeString();
        m_termGen->index_text_without_positions(str.toStdString(), 1, "S");
        m_termGen->index_text_without_positions(str.toStdString());
    }

    KMime::Headers::Date* date = msg->date(false);
    if (date) {
        QString str = date->dateTime().toString();
        m_doc->add_value(0, str.toStdString());
    }

    insert("F", msg->from(false));
    insert("T", msg->to(false));
    insert("CC", msg->cc(false));
    insert("BC", msg->bcc(false));

    // Stuff that could be indexed
    // - Message ID
    // - Organization
    // - listID
    // - mailingList

    //
    // Process Plain Text Content
    //
    if (msg->contents().isEmpty())
        return;

    KMime::Content* mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
        const std::string text = mainBody->decodedText().toStdString();
        m_termGen->index_text_without_positions(text);
        // Maybe we want to index the text twice?
    }

    if (!mainBody)
        processPart(msg.get(), mainBody);
}

void EmailIndexer::processPart(KMime::Content* content, KMime::Content* mainContent)
{
    if (content == mainContent) {
        return;
    }

    KMime::Headers::ContentType* type = content->contentType(false);
    if (type && type->isMultipart()) {
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

        const std::string text = doc.toPlainText().toStdString();
        m_termGen->index_text_without_positions(text);
    }

    // FIXME: Handle attachments?
}

void EmailIndexer::processMessageStatus(const Akonadi::MessageStatus& status)
{
    insertBool('R', status.isRead());
    insertBool('A', status.hasAttachment());
    insertBool('I', status.isImportant());
    insertBool('W', status.isWatched());
}

void EmailIndexer::insertBool(char key, bool value)
{
    QByteArray term("B");
    if (value) {
        term.append(key);
    }
    else {
        term.append('N');
        term.append(key);
    }

    m_doc->add_boolean_term(term.data());
}


void EmailIndexer::commit()
{
    m_db->commit();
}
