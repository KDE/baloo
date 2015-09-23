/*
   Copyright (c) 2012-2013 Vishesh Handa <me@vhanda.in>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>

#include <KAboutData>
#include <KLocalizedString>

#include <QJsonDocument>
#include <QJsonObject>

#include "global.h"
#include "idutils.h"
#include "database.h"
#include "transaction.h"

#include <KFileMetaData/PropertyInfo>


QString colorString(const QString& input, int color)
{
    QString colorStart = QString::fromLatin1("\033[0;%1m").arg(color);
    QLatin1String colorEnd("\033[0;0m");

    return colorStart + input + colorEnd;
}

int main(int argc, char* argv[])
{
    KAboutData aboutData(QLatin1String("balooshow"),
                         i18n("Baloo Show"),
                         PROJECT_VERSION,
                         i18n("The Baloo data Viewer - A debugging tool"),
                         KAboutLicense::GPL,
                         i18n("(c) 2012, Vishesh Handa"));
    aboutData.addAuthor(i18n("Vishesh Handa"), i18n("Maintainer"), QLatin1String("me@vhanda.in"));

    KAboutData::setApplicationData(aboutData);
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addPositionalArgument(QLatin1String("files"), QLatin1String("The file urls"));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("x"),
                                        QLatin1String("Print internal info")));
    parser.addHelpOption();
    parser.process(app);

    QStringList args = parser.positionalArguments();

    if (args.isEmpty()) {
        parser.showHelp(1);
    }

    //
    // File Urls
    //
    QStringList urls;
    Q_FOREACH (const QString& arg, args) {
        const QString url = QFileInfo(arg).absoluteFilePath();
        if (QFile::exists(url)) {
            urls.append(url);
        } else {
            urls.append(QLatin1String("file:") + arg);
        }
    }

    QTextStream stream(stdout);
    QString text;

    Baloo::Database *db = Baloo::globalDatabaseInstance();
    if (!db->open(Baloo::Database::OpenDatabase)) {
        stream << i18n("The Baloo index could not be opened. Please run \"balooctl status\" to see if Baloo is enabled and working.")
               << endl;
        return 1;
    }

    Baloo::Transaction tr(db, Baloo::Transaction::ReadOnly);

    for (QString url : urls) {
        quint64 fid = 0;
        if (url.startsWith(QLatin1String("file:"))) {
            fid = url.mid(5).toULongLong();
            url = QFile::decodeName(tr.documentUrl(fid));

            // Debugging aid
            quint64 actualFid = Baloo::filePathToId(QFile::encodeName(url));
            if (fid != actualFid) {
                stream << i18n("The fileID is not equal to the actual Baloo fileID") << endl;
                stream << i18n("This is a bug") << endl;

                stream << "GivenID: " << fid << " ActualID: " << actualFid << "\n";
                stream << "GivenINode: " << Baloo::idToInode(fid) << " ActualINode: " << Baloo::idToInode(actualFid) << "\n";
                stream << "GivenDeviceID: " << Baloo::idToDeviceId(fid) << " ActualDeviceID: " << Baloo::idToDeviceId(actualFid) << "\n";
            }
        } else {
            fid = Baloo::filePathToId(QFile::encodeName(url));
        }

        bool hasFile = tr.hasDocument(fid);
        if (hasFile) {
            text = colorString(QString::number(fid), 31);
            text += QLatin1String(" ");
            text += colorString(QString::number(Baloo::idToDeviceId(fid)), 28);
            text += QLatin1String(" ");
            text += colorString(QString::number(Baloo::idToInode(fid)), 28);
            text += QLatin1String(" ");
            text += colorString(url, 32);
            stream << text << endl;
        }
        else {
            stream << i18n("No index information found") << endl;
            continue;
        }

        const QJsonDocument jdoc = QJsonDocument::fromJson(tr.documentData(fid));
        const QVariantMap varMap = jdoc.object().toVariantMap();
        KFileMetaData::PropertyMap propMap = KFileMetaData::toPropertyMap(varMap);
        KFileMetaData::PropertyMap::const_iterator it = propMap.constBegin();
        for (; it != propMap.constEnd(); ++it) {
            QString str;
            if (it.value().type() == QVariant::List) {
                QStringList list;
                for (const QVariant& var : it.value().toList()) {
                    list << var.toString();
                }
                str = list.join(", ");
            } else {
                str = it.value().toString();
            }

            KFileMetaData::PropertyInfo pi(it.key());
            stream << "\t" << pi.displayName() << ": " << str << endl;
        }

        if (parser.isSet(QStringLiteral("x"))) {
            QVector<QByteArray> terms = tr.documentTerms(fid);
            QVector<QByteArray> fileNameTerms = tr.documentFileNameTerms(fid);
            QVector<QByteArray> xAttrTerms = tr.documentXattrTerms(fid);

            auto join = [](const QVector<QByteArray>& v) {
                QByteArray ba;
                for (const QByteArray& arr : v) {
                    ba.append(arr);
                    ba.append(' ');
                }
                return ba;
            };

            stream << "\nInternal Info\n";
            stream << "Terms: " << join(terms) << "\n";
            stream << "File Name Terms: " << join(fileNameTerms) << "\n";
            stream << "XAttr Terms: " << join(xAttrTerms) << "\n\n";

            QHash<int, QStringList> propertyWords;

            for (const QByteArray& arr : terms) {
                QString word = QString::fromUtf8(arr);

                if (word[0].isUpper()) {
                    if (word[0] == QLatin1Char('X')) {
                        int posOfNonNumeric = 1;
                        while (word[posOfNonNumeric] != '-') {
                            posOfNonNumeric++;
                        }

                        int propNum = word.mid(1, posOfNonNumeric-1).toInt();
                        QString value = word.mid(posOfNonNumeric + 1);

                        propertyWords[propNum].append(value);
                    }
                }
            }

            for (auto it = propertyWords.constBegin(); it != propertyWords.constEnd(); it++) {
                auto prop = static_cast<KFileMetaData::Property::Property>(it.key());
                KFileMetaData::PropertyInfo pi(prop);

                stream << pi.name() << ": " << it.value().join(" ") << endl;
            }
        }
    }

    return 0;
}
