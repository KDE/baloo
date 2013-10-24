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

#include <QTextDocument>

EmailIndexer::EmailIndexer()
{
    m_db.setPath("/tmp/xap/");
}

EmailIndexer::~EmailIndexer()
{
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

    m_db.beginDocument(item.id());
    processMessageStatus(status);

    KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
    process(msg);

    m_db.endDocument();
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

void EmailIndexer::insert(const QByteArray& key, const KMime::Types::Mailbox::List& list)
{
    Q_FOREACH (const KMime::Types::Mailbox& mbox, list) {
        m_db.appendText(key, mbox.name());
        m_db.appendText("all", mbox.name());

        m_db.append(key, mbox.address());
        m_db.append("all", mbox.address());
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
        m_db.setText("subject", subject->asUnicodeString());
        m_db.appendText("all", subject->asUnicodeString());
    }

    KMime::Headers::Date* date = msg->date(false);
    if (date) {
        // FIXME: We cannot index the date as a string! We need it for sorting!
        // m_db.insert("date", date->dateTime().toString());
    }

    insert("from", msg->from(false));
    insert("to", msg->to(false));
    insert("cc", msg->cc(false));
    insert("bcc", msg->bcc(false));

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
        const QString text = mainBody->decodedText();
        m_db.appendText("text", text);
        m_db.appendText("all", text);
    }
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

        m_db.appendText("body", doc.toPlainText());
        m_db.appendText("all", doc.toPlainText());
    }

    // FIXME: Handle attachments?
}

void EmailIndexer::processMessageStatus(const Akonadi::MessageStatus& status)
{
    m_db.appendBool("all", "isRead", status.isRead());
    m_db.appendBool("all", "hasAttachment", status.hasAttachment());
    m_db.appendBool("all", "isImportant", status.isImportant());

    // FIXME: How do we deal with the other flags?
}

void EmailIndexer::commit()
{
    m_db.commit();
}
