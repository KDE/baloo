/*
 * This file is part of the KDE Baloo project.
 * Copyright 2018  Michael Heidelbach <ottwolt@gmail.com>
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
 */

#include "databasesanitizer.h"
#include "documenturldb.h"
#include "baloodebug.h"

#include <KLocalizedString>
#include <QFileInfo>

namespace Baloo
{
    
class DatabaseSanitizerImpl {
public:
    DatabaseSanitizerImpl(const Database& db, Transaction::TransactionType type)
    : m_transaction(new Transaction(db, type))
    {
    }

public:
    /**
    * \brief Basic info about database items
    */
    struct FileInfo {
        quint32 deviceId = 0; 
        quint32 inode = 0;
        QString url = QString();
        bool accessible = true;
    };
    
    void printProgress(QTextStream& out, uint& cur, const uint max, const uint step) const
    {
        if (cur % step == 0) {
            out << QStringLiteral("%1%2\r").arg(100 * cur / max, 6).arg("%", -16);
            out.flush();
        }
        cur++;
    }
    
    /**
    * Create a list of \a FileInfo items.
    * 
    * \p deviceIDs filter by device ids. If the vector is empty no filtering is done
    * and every item is collected.
    * Positive numbers are including filters collecting only the mentioned device ids.
    * Negative numbers are excluding filters collecting everything but the mentioned device ids.
    * 
    * \p missingOnly Only inaccessible items are collected.
    * 
    * \p urlFilter Filter result urls. Default is null = Collect everything.
    */
    QVector<FileInfo> createList(
        const QVector<qint64>& deviceIds, 
        const bool purging, 
        const QSharedPointer<QRegularExpression>& urlFilter
    ) const
    {
        Q_ASSERT(m_transaction);

        const auto docUrlDb = DocumentUrlDB(m_transaction->m_dbis.idTreeDbi, 
                                            m_transaction->m_dbis.idFilenameDbi, 
                                            m_transaction->m_txn);
        const auto map = docUrlDb.toTestMap();
        const auto keys = map.keys();
        QVector<FileInfo> result;
        result.reserve(keys.count());
        uint i = 0;
        uint max = keys.count();
        QVector<quint32> includeIds;
        QVector<quint32> excludeIds;
        for (qint64 deviceId : deviceIds) {
            if (deviceId > 0) {
                includeIds.append(deviceId);
            } else if (deviceId < 0) {
                excludeIds.append(-deviceId);
            }
        }
        
        QTextStream err(stderr);
        for (quint64 id: keys) {
            printProgress(err, i, max, 100);
            
            const quint32* arr = reinterpret_cast<quint32*>(&id);
            const auto url = docUrlDb.get(id);
            FileInfo info;
            info.deviceId = arr[0];
            info.inode = arr[1];
            info.url = url;
            info.accessible = !url.isEmpty() && QFileInfo::exists(url);
            if ((!includeIds.isEmpty() && !includeIds.contains(info.deviceId))
                || (!excludeIds.isEmpty() && excludeIds.contains(info.deviceId))
                || (purging && info.accessible)
                || (urlFilter && !urlFilter->match(info.url).hasMatch())
            ) {
                continue;
            }
            result.append(info);
        }
        return result;
    }
    
    QMultiHash<quint32, FileInfo> createDeviceList(const QVector<qint64>& deviceIds)
    {
        auto infos = createList(deviceIds, false, nullptr);
        QMultiHash<quint32, FileInfo> usedDevices;
        for (const auto& info: infos) {
            usedDevices.insert(info.deviceId, info);
        }
        return usedDevices;
    }
    
private:
    Transaction* m_transaction;
};
}

using namespace Baloo;


DatabaseSanitizer::DatabaseSanitizer(const Database& db, Baloo::Transaction::TransactionType type)
    : m_pimpl(new DatabaseSanitizerImpl(db, type))
{
}

DatabaseSanitizer::DatabaseSanitizer(Database* db, Transaction::TransactionType type)
    : DatabaseSanitizer(*db, type)
{
}

DatabaseSanitizer::~DatabaseSanitizer()
{
    delete m_pimpl;
    m_pimpl = nullptr;
}

/**
* Create a list of \a FileInfo items and print it to stdout.
* 
* \p deviceIDs filter by device ids. If the vector is empty no filtering is done
* and everything is printed.
* Positive numbers are including filters printing only the mentioned device ids.
* Negative numbers are excluding filters printing everything but the mentioned device ids.
* 
* \p missingOnly Simulate purging operation. Only inaccessible items are printed.
* 
* \p urlFilter Filter result urls. Default is null = Print everything.
*/
 void DatabaseSanitizer::printList(
    const QVector<qint64>& deviceIds, 
    const bool missingOnly, 
    const QSharedPointer<QRegularExpression>& urlFilter)
{
    auto infos = m_pimpl->createList(deviceIds, missingOnly, urlFilter);
    const auto sep = QLatin1Char(' ');
    QTextStream out(stdout);
    QTextStream err(stderr);
    for (const auto& info: infos) {
        if (!missingOnly) {
            out << QStringLiteral("%1").arg(info.accessible ? "+" : "!") << sep;
        } else if (!info.accessible) {
            out << i18n("Missing:") << sep;
        } else {
            Q_ASSERT(false);
            continue;
        }
        out << QStringLiteral("device: %1").arg(info.deviceId)
        << sep << QStringLiteral("inode: %1").arg(info.inode)
        << sep << QStringLiteral("url: %1").arg(info.url)
        << endl;
    }
    err << i18n("Found %1 matching items", infos.count()) << endl;
    
}

void DatabaseSanitizer::printDevices(const QVector<qint64>& deviceIds, const bool missingOnly)
{
    Q_UNUSED(missingOnly)
    /* 
     * TODO: Implement missingOnly filter. Checking for file existence
     * will not work. We need to read /etc/mtab or so.
     */
    auto usedDevices = m_pimpl->createDeviceList(deviceIds);
    
    const auto sep = QLatin1Char(' ');
    QTextStream out(stdout);
    QTextStream err(stderr);
    
    for (const auto& dev: usedDevices.uniqueKeys()) {
        out << "Device:" << dev 
        << sep  << usedDevices.values(dev).count() << sep << "items" 
        << endl;
    }
    
    err << i18n("Found %1 matching items", usedDevices.count()) << endl;
}
