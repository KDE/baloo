/*
 * This file is part of the KDE Baloo project.
 * Copyright (C) 2015  Vishesh Handa <vhanda@kde.org>
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

#include "statuscommand.h"
#include "indexerconfig.h"

#include "global.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include "schedulerinterface.h"
#include "maininterface.h"
#include "indexerstate.h"

#include <KLocalizedString>
#include <KFormat>

using namespace Baloo;

QString StatusCommand::command()
{
    return QStringLiteral("status");
}

QString StatusCommand::description()
{
    return i18n("Print the status of the Indexer");
}

class FileIndexStatus
{
public:
    enum class FileStatus : uint8_t {
        NonExisting,
        Directory,
        RegularFile,
        SymLink,
        Other,
    };
    enum class IndexStateReason : uint8_t {
        NoFileOrDirectory,
        ExcludedByPath, // FIXME - be more specific, requires changes to shouldBeIndexed(path)
        WaitingForIndexingBoth,
        WaitingForBasicIndexing,
        BasicIndexingDone,
        WaitingForContentIndexing,
        FailedToIndex,
        Done,
    };
    QString m_filePath;
    FileStatus m_fileStatus;
    IndexStateReason m_indexState;
    uint32_t m_dataSize;
};

FileIndexStatus collectFileStatus(Transaction& tr, IndexerConfig&  cfg, const QString& file)
{
    using FileStatus = FileIndexStatus::FileStatus;
    using IndexStateReason = FileIndexStatus::IndexStateReason;

    bool onlyBasicIndexing = cfg.onlyBasicIndexing();

    QFileInfo fileInfo = QFileInfo(file);
    QString filePath = fileInfo.absoluteFilePath();
    quint64 id = filePathToId(QFile::encodeName(filePath));
    if (id == 0) {
        return FileIndexStatus{filePath, FileStatus::NonExisting, IndexStateReason::NoFileOrDirectory, 0};
    }

    FileStatus fileStatus = fileInfo.isSymLink() ? FileStatus::SymLink :
                            fileInfo.isFile() ? FileStatus::RegularFile :
                            fileInfo.isDir() ? FileStatus::Directory : FileStatus::Other;

    if (fileStatus == FileStatus::Other || fileStatus == FileStatus::SymLink) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::NoFileOrDirectory, 0};
    }

    if (!cfg.shouldBeIndexed(filePath)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::ExcludedByPath, 0};
    }

    if (onlyBasicIndexing || fileStatus == FileStatus::Directory) {
        if (!tr.hasDocument(id)) {
            return FileIndexStatus{filePath, fileStatus, IndexStateReason::WaitingForBasicIndexing, 0};
        } else {
            return FileIndexStatus{filePath, fileStatus, IndexStateReason::BasicIndexingDone, 0};
        }
    }

    // File && shouldBeIndexed && contentIndexing
    if (!tr.hasDocument(id)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::WaitingForIndexingBoth, 0};
    } else if (tr.inPhaseOne(id)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::WaitingForContentIndexing, 0};
    } else if (tr.hasFailed(id)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::FailedToIndex, 0};
    } else {
        uint32_t size = tr.documentData(id).size();
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::Done, size};
    }
}

void printMultiLine(Transaction& tr, IndexerConfig&  cfg, const QStringList& args) {
    QTextStream out(stdout);
    QTextStream err(stderr);
    for (const QString& arg : args) {
      QString filePath = QFileInfo(arg).absoluteFilePath();
      quint64 id = filePathToId(QFile::encodeName(filePath));
      if (id == 0) {
        err << i18n("Ignoring non-existent file %1", filePath) << endl;
        continue;
      }


      out << i18n("File: %1", filePath) << endl;
      if (tr.hasDocument(id)) {
          out << i18n("Basic Indexing: Done") << endl;
      } else if (cfg.shouldBeIndexed(filePath)) {
          out << i18n("Basic Indexing: Scheduled") << endl;
          continue;
      } else {
          // FIXME: Add why it is not being indexed!
          out << i18n("Basic Indexing: Disabled") << endl;
          continue;
      }

      if (QFileInfo(arg).isDir()) {
          continue;
      }

      if (tr.inPhaseOne(id)) {
          out << i18n("Content Indexing: Scheduled") << endl;
      } else if (tr.hasFailed(id)) {
          out << i18n("Content Indexing: Failed") << endl;
      } else {
          out << i18n("Content Indexing: Done") << endl;
      }
  }
}

void printSimpleFormat(Transaction& tr, IndexerConfig&  cfg, const QStringList& args) {
    using FileStatus = FileIndexStatus::FileStatus;
    using IndexStateReason = FileIndexStatus::IndexStateReason;

    QTextStream out(stdout);
    QTextStream err(stderr);

    const QMap<IndexStateReason, QString> simpleIndexStateValue = {
        { IndexStateReason::NoFileOrDirectory,         QStringLiteral("No regular file or directory") },
        { IndexStateReason::ExcludedByPath,            QStringLiteral("Indexing disabled") },
        { IndexStateReason::WaitingForIndexingBoth,    QStringLiteral("Basic and Content indexing scheduled") },
        { IndexStateReason::WaitingForBasicIndexing,   QStringLiteral("Basic indexing scheduled") },
        { IndexStateReason::BasicIndexingDone,         QStringLiteral("Basic indexing done") },
        { IndexStateReason::WaitingForContentIndexing, QStringLiteral("Content indexing scheduled") },
        { IndexStateReason::FailedToIndex,             QStringLiteral("Content indexing failed") },
        { IndexStateReason::Done,                      QStringLiteral("Content indexing done") },
    };

    for (const auto& fileName : args) {
        const auto file = collectFileStatus(tr, cfg, fileName);

        if (file.m_fileStatus == FileStatus::NonExisting) {
            err << i18n("Ignoring non-existent file %1", file.m_filePath) << endl;
            continue;
        }

        if (file.m_fileStatus == FileStatus::SymLink || file.m_fileStatus == FileStatus::Other) {
            err << i18n("Ignoring symlink/special file %1", file.m_filePath) << endl;
            continue;
        }

        out << simpleIndexStateValue[file.m_indexState];
        out << ": " << file.m_filePath << endl;
    }
}

void printJSON(Transaction& tr, IndexerConfig&  cfg, const QStringList& args) {

  QJsonArray filesInfo;
  for (const QString& arg : args) {
      QString filePath = QFileInfo(arg).absoluteFilePath();
      quint64 id = filePathToId(QFile::encodeName(filePath));

      if (id == 0) {
          QTextStream err(stderr);
          err << i18n("Ignoring non-existent file %1", filePath) << endl;
          continue;
      }
      QJsonObject fileInfo;
      fileInfo["file"] = filePath;
      if (!tr.hasDocument(id)) {
          fileInfo["indexing"] = "basic";
          if (cfg.shouldBeIndexed(filePath)) {
              fileInfo["status"] = "scheduled";
          } else {
              fileInfo["status"] = "disabled";
          }

      } else if (QFileInfo(arg).isDir()) {
          fileInfo["indexing"] = "basic";
          fileInfo["status"] =  "done";

      } else {
          fileInfo["indexing"] = "content";
          if (tr.inPhaseOne(id)) {
              fileInfo["status"] = "scheduled";
          } else if (tr.hasFailed(id)) {
              fileInfo["status"] = "failed";
          } else {
              fileInfo["status"] = "done";
          }
      }

      filesInfo.append(fileInfo);
  }

  QJsonDocument json;
  json.setArray(filesInfo);
  QTextStream out(stdout);
  out << json.toJson(QJsonDocument::Indented);
}

int StatusCommand::exec(const QCommandLineParser& parser)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    const QStringList allowedFormats({"simple", "json", "multiline"});
    const QString format = parser.value(QStringLiteral("format"));

    if (!allowedFormats.contains(format)) {
        err << i18n("Output format \"%1\" is invalid", format) << endl;
        return 1;
    }

    IndexerConfig cfg;
    if (!cfg.fileIndexingEnabled()) {
        err << i18n("Baloo is currently disabled. To enable, please run %1", QStringLiteral("balooctl enable")) << endl;
        return 1;
    }

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadOnlyDatabase)) {
        err << i18n("Baloo Index could not be opened") << endl;
        return 1;
    }

    Transaction tr(db, Transaction::ReadOnly);

    QStringList args = parser.positionalArguments();
    args.pop_front();

    if (args.isEmpty()) {
        org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                    QStringLiteral("/"),
                                                    QDBusConnection::sessionBus());

        org::kde::baloo::scheduler schedulerinterface(QStringLiteral("org.kde.baloo"),
                                            QStringLiteral("/scheduler"),
                                            QDBusConnection::sessionBus());

        bool running = mainInterface.isValid();

        if (running) {
            out << i18n("Baloo File Indexer is running") << endl;
            out << i18n("Indexer state: %1", stateString(schedulerinterface.state())) << endl;
        }
        else {
            out << i18n("Baloo File Indexer is not running") << endl;
        }

        uint phaseOne = tr.phaseOneSize();
        uint total = tr.size();

        out << i18n("Indexed %1 / %2 files", total - phaseOne, total) << endl;

        const QString path = fileIndexDbPath();

        const QFileInfo indexInfo(path + QLatin1String("/index"));
        const auto size = indexInfo.size();
        KFormat format(QLocale::system());
        if (size) {
            out << i18n("Current size of index is %1", format.formatByteSize(size, 2)) << endl;
        } else {
            out << i18n("Index does not exist yet") << endl;
        }
    } else if (format == allowedFormats[0]){
        printSimpleFormat(tr, cfg, args);
    } else if (format == allowedFormats[1]){
        printJSON(tr, cfg, args);
    } else {
        printMultiLine(tr, cfg, args);
    }

    return 0;
}
