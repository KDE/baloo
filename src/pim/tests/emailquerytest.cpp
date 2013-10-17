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

#include <QCoreApplication>
#include <QTimer>
#include <KDebug>

#include <xapian.h>

class App : public QCoreApplication {
    Q_OBJECT
public:
    App(int& argc, char** argv, int flags = ApplicationFlags);

    QString m_query;

private slots:
    void main();
};

int main(int argc, char** argv)
{
    App app(argc, argv);

    if (argc != 2) {
        kError() << "Proper args required";
    }
    app.m_query = argv[1];

    return app.exec();
}

App::App(int& argc, char** argv, int flags): QCoreApplication(argc, argv, flags)
{
    QTimer::singleShot(0, this, SLOT(main()));
}

void App::main()
{
    QString base("/tmp/xap/");
    Xapian::Database databases;
    databases.add_database(Xapian::Database((base + "text").toStdString()));

    Xapian::Enquire enquire(databases);
    //Xapian::Query query("aleix");

    QTime timer;
    timer.start();
    Xapian::QueryParser queryParser;
    queryParser.set_database(databases);
    Xapian::Query query = queryParser.parse_query(m_query.toStdString(), Xapian::QueryParser::FLAG_PARTIAL);

    kDebug() << "Query:" << query.get_description().c_str();
    enquire.set_query(query);
    Xapian::MSet matches = enquire.get_mset(0, 10000);
    Xapian::MSetIterator i;
    for (i = matches.begin(); i != matches.end(); ++i) {
        //kDebug() << "Document ID " << *i << "\t";
        kDebug() << "Document ID " << i.get_document().get_docid() << "\t";
        /*kDebug() << i.get_percent() << "% ";
        Xapian::Document doc = i.get_document();
        kDebug() << "[" << doc.get_data().c_str() << "]" << endl;*/
    }
    kDebug() << matches.size();
    kDebug() << "Elapsed:" << timer.elapsed();
    quit();
}



#include "emailquerytest.moc"
