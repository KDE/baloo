/*
 * This File is part of the KDE Baloo Project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "filestatistics.h"

#include <QHashIterator>
#include <QDebug>

using namespace Baloo;

FileStatistics::FileStatistics(XapianDatabase& xapDb)
    : m_xapDb(xapDb)
    , m_totalNumDocuments(0)
    , m_totalNumTerms(0)
{
}

void FileStatistics::compute()
{
    qDebug() << "Computing";
    Xapian::Database* db = m_xapDb.db();
    m_totalNumDocuments = db->get_doccount();

    Xapian::Enquire enq(*db);
    enq.set_query(Xapian::Query(std::string()));

    Xapian::MSet mset = enq.get_mset(0, m_totalNumDocuments);
    Xapian::MSetIterator it = mset.begin();
    for (; it != mset.end(); it++) {
        Xapian::Document doc = it.get_document();
        auto tit = doc.termlist_begin();

        // Get mimetype
        tit.skip_to("M");
        std::string mimetypeStr = *tit;
        QString mimetype = QString::fromUtf8(mimetypeStr.c_str(), mimetypeStr.length()).mid(1);

        m_typeInfo[mimetype].mimetype = mimetype;
        tit = doc.termlist_begin();
        for (; tit != doc.termlist_end(); tit++) {
            m_typeInfo[mimetype].numTerms++;
        }

        m_typeInfo[mimetype].numDocuments++;
    }

    m_totalNumDocuments = 0;
    m_totalNumTerms = 0;

    QHashIterator<QString, MimeTypeInfo> hit(m_typeInfo);
    while (hit.hasNext()) {
        hit.next();

        m_totalNumDocuments += hit.value().numDocuments;
        m_totalNumTerms += hit.value().numTerms;
    }
}

namespace {
    QString colorString(const QString& input, int color)
    {
        QString colorStart = QString::fromLatin1("\033[0;%1m").arg(color);
        QLatin1String colorEnd("\033[0;0m");

        return colorStart + input + colorEnd;
    }

}
void FileStatistics::print()
{
    QList<MimeTypeInfo> infos = m_typeInfo.values();

    // Reverse sort
    auto func = [](const MimeTypeInfo& l, const MimeTypeInfo& r) {
        return l.numTerms > r.numTerms;
    };
    qSort(infos.begin(), infos.end(), func);

    QTextStream os(stdout);
    Q_FOREACH (const MimeTypeInfo& info, infos) {
        float percent = (info.numTerms * 100.0) / m_totalNumTerms;

        int avgNumTerms = info.numTerms / info.numDocuments;

        QString out;
        out += '[';
        out += colorString(QString::number(percent), 31);
        out += "]\t";
        out += colorString(info.mimetype, 32);
        out += " -> ";
        out += colorString(QString::number(info.numDocuments), 44);
        out += ' ';
        out += colorString(QString::number(avgNumTerms), 31);

        os << out << "\n";
    }
}
