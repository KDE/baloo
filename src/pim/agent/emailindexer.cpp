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

#include "emailindexer.h"

#include <Akonadi/Collection>
#include <Akonadi/KMime/MessageFlags>

#include <QTextDocument>

EmailIndexer::EmailIndexer(const QString& path, const QString& contactDbPath):
    AbstractIndexer(), m_termGen( 0 )
{
    m_db = new Xapian::WritableDatabase(path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
    m_contactDb = new Xapian::WritableDatabase(contactDbPath.toStdString(), Xapian::DB_CREATE_OR_OPEN);
}

EmailIndexer::~EmailIndexer()
{
    m_db->commit();
    delete m_db;

    m_contactDb->commit();
    delete m_contactDb;
}

QStringList EmailIndexer::mimeTypes() const
{
    return QStringList() << KMime::Message::mimeType();
}

void EmailIndexer::index(const Akonadi::Item& item)
{
    Akonadi::MessageStatus status;
    status.setStatusFromFlags(item.flags());
    if (status.isSpam())
        return;

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

    processMessageStatus(status);
    process(msg);

    // Size
    m_doc->add_value(1, QString::number(item.size()).toStdString());

    // Parent collection
    Q_ASSERT_X(item.parentCollection().isValid(), "Baloo::EmailIndexer::index",
               "Item does not have a valid parent collection");

    Akonadi::Entity::Id colId = item.parentCollection().id();
    QByteArray term = 'C' + QByteArray::number(colId);
    m_doc->add_boolean_term(term.data());

    m_db->replace_document(item.id(), *m_doc);

    delete m_doc;
    delete m_termGen;

    m_doc = 0;
    m_termGen = 0;
}

void EmailIndexer::insert(const QByteArray& key, KMime::Headers::Base* unstructured)
{
    if (unstructured) {
        m_termGen->index_text_without_positions(unstructured->asUnicodeString().toUtf8().constData(), 1, key.data());
    }
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

namespace {
    // Does some extra stuff such as lower casing the email, removing all quotes
    // and removing extra spaces
    // TODO: Move this into KMime?
    // TODO: If name is all upper/lower then try to captialize it?
    QString prettyAddress(const KMime::Types::Mailbox& mbox) {
        QString name = mbox.name().simplified();
        QByteArray email = mbox.address().simplified().toLower();

        // Remove outer quotes recursively
        while (name.size() >= 2 && (name[0] == '\'' || name[0] == '"') &&
               (name[name.size()-1] == '\'' || name[name.size()-1] == '"')) {
            name = name.mid(1, name.size()-2);
        }

        if (name.isEmpty())
            return email;
        else
            return name + QLatin1String(" <") + QString::fromUtf8(email) + QLatin1Char('>');
    }
}

// Add once with a prefix and once without
void EmailIndexer::insert(const QByteArray& key, const KMime::Types::Mailbox::List& list)
{
    Q_FOREACH (const KMime::Types::Mailbox& mbox, list) {
        std::string name(mbox.name().toUtf8().constData());
        m_termGen->index_text_without_positions(name, 1, key.data());
        m_termGen->index_text_without_positions(name, 1);

        m_doc->add_term((key + mbox.address()).data());
        m_doc->add_term(mbox.address().data());

        //
        // Add emails for email auto-completion
        //
        const QString pa = prettyAddress(mbox);
        int id = qHash(pa);
        try {
            Xapian::Document doc = m_contactDb->get_document(id);
            continue;
        }
        catch (const Xapian::DocNotFoundError&) {
            Xapian::Document doc;
            std::string pretty(pa.toUtf8().constData());
            doc.set_data(pretty);

            Xapian::TermGenerator termGen;
            termGen.set_document(doc);
            termGen.index_text(pretty);

            doc.add_term(mbox.address().data());
            m_contactDb->replace_document(id, doc);
        }
    }
}

// FIXME: Only index properties that are actually searched!
void EmailIndexer::process(const KMime::Message::Ptr& msg)
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

    KMime::Headers::Date* date = msg->date(false);
    if (date) {
        const QString str = QString::number(date->dateTime().toTime_t());
        m_doc->add_value(0, str.toStdString());
        const QString julianDay = QString::number(date->dateTime().date().toJulianDay());
        m_doc->add_value(2, julianDay.toStdString());
    }

    insert("F", msg->from(false));
    insert("T", msg->to(false));
    insert("CC", msg->cc(false));
    insert("BC", msg->bcc(false));
    insert("O", msg->organization(false));
    insert("RT", msg->replyTo(false));
    insert("RF", msg->headerByType("Resent-From"));
    insert("LI", msg->headerByType("List-Id"));
    insert("XL", msg->headerByType("X-Loop"));
    insert("XML", msg->headerByType("X-Mailing-List"));
    insert("XSF", msg->headerByType("X-Spam-Flag"));

    //
    // Process Plain Text Content
    //

    //Index all headers
    m_termGen->index_text_without_positions(std::string(msg->head().constData()), 1, "HE");

    KMime::Content* mainBody = msg->mainBodyPart("text/plain");
    if (mainBody) {
        const std::string text(mainBody->decodedText().toUtf8().constData());
        m_termGen->index_text_without_positions(text);
        m_termGen->index_text_without_positions(text, 1, "BO");
    }
    else {
        processPart(msg.get(), 0);
    }
}

void EmailIndexer::processPart(KMime::Content* content, KMime::Content* mainContent)
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

    // FIXME: Handle attachments?
}

void EmailIndexer::processMessageStatus(const Akonadi::MessageStatus& status)
{
    insertBool('R', status.isRead());
    insertBool('A', status.hasAttachment());
    insertBool('I', status.isImportant());
    insertBool('W', status.isWatched());
    insertBool('T', status.isToAct());
    insertBool('D', status.isDeleted());
    insertBool('S', status.isSpam());
    insertBool('E', status.isReplied());
    insertBool('G', status.isIgnored());
    insertBool('F', status.isForwarded());
    insertBool('N', status.isSent());
    insertBool('Q', status.isQueued());
    insertBool('H', status.isHam());
    insertBool('C', status.isEncrypted());
    insertBool('V', status.hasInvitation());
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

void EmailIndexer::toggleFlag(Xapian::Document& doc, const char* remove, const char* add)
{
    try {
        doc.remove_term(remove);
    }
    catch (const Xapian::InvalidArgumentError &e) {
        // The previous flag state was not indexed, continue
    }

    doc.add_term(add);
}


void EmailIndexer::updateFlags(const Akonadi::Item& item,
                               const QSet<QByteArray>& added,
                               const QSet<QByteArray>& removed)
{
    Xapian::Document doc;
    try {
        doc = m_db->get_document(item.id());
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }

    Q_FOREACH (const QByteArray& flag, removed) {
        if (flag == Akonadi::MessageFlags::Seen) {
            toggleFlag(doc, "BR", "BNR");
        }
        else if (flag == Akonadi::MessageFlags::Flagged) {
            toggleFlag(doc, "BI", "BNI");
        }
        else if (flag == Akonadi::MessageFlags::Watched) {
            toggleFlag(doc, "BW", "BNW");
        }
    }

    Q_FOREACH (const QByteArray& flag, added) {
        if (flag == Akonadi::MessageFlags::Seen) {
            toggleFlag(doc, "BNR", "BR");
        }
        else if (flag == Akonadi::MessageFlags::Flagged) {
            toggleFlag(doc, "BNI", "BI");
        }
        else if (flag == Akonadi::MessageFlags::Watched) {
            toggleFlag(doc, "BNW", "BW");
        }
    }

    m_db->replace_document(doc.get_docid(), doc);
}

void EmailIndexer::remove(const Akonadi::Item& item)
{
    try {
        m_db->delete_document(item.id());
        //TODO remove contacts from contact db?
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void EmailIndexer::remove(const Akonadi::Collection& collection)
{
    try {
        Xapian::Query query('C'+ QString::number(collection.id()).toStdString());
        Xapian::Enquire enquire(*m_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, m_db->get_doccount());
        Xapian::MSetIterator end = mset.end();
        for (Xapian::MSetIterator it = mset.begin(); it != end; ++it) {
            const qint64 id = *it;
            remove(Akonadi::Item(id));
        }
    }
    catch (const Xapian::DocNotFoundError&) {
        return;
    }
}

void EmailIndexer::move(const Akonadi::Item::Id& itemId,
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


void EmailIndexer::commit()
{
    m_db->commit();
    m_contactDb->commit();
}
