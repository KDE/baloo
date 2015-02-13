#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

#include <iostream>

#include "luceneindex.cpp"
#include "lucenedocument.cpp"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << "db", "database", "path to database"));
    parser.addPositionalArgument("query", "word to search for");
    parser.process(app);

    QStringList query = parser.positionalArguments();
    QString path = parser.value("db");
    LuceneIndex index(path);
    Lucene::IndexReaderPtr reader = index.IndexReader();
    Lucene::SearcherPtr searcher = Lucene::newLucene<Lucene::IndexSearcher>(reader);
    Lucene::TermQueryPtr termQuery = Lucene::newLucene<Lucene::TermQuery>( LuceneIndex::makeTerm("content", query.at(0)) );
    Lucene::TopDocsPtr topDocs = searcher->search( termQuery, 10 );
    auto scoreDocs = topDocs->scoreDocs;
    QTextStream out(stdout);
    for (auto it = scoreDocs.begin(); it != scoreDocs.end(); ++it) {
        LuceneDocument doc(reader->document((*it)->doc));
        out << doc.getFieldValues("URL").at(0) << endl;
    }
}