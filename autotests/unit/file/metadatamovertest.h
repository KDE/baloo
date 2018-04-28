#if !defined METADATAMOVERTEST_H_
#define METADATAMOVERTEST_H_

#include <QObject>
#include <QStringList>
#include <QString>

class MetadataMoverTestDBusSpy : public QObject
{
    Q_OBJECT
public:

    MetadataMoverTestDBusSpy(QObject* parent = nullptr);

Q_SIGNALS:

    void fileMetaDataChanged(QStringList fileList);

    void renamedFilesSignal(const QString &from, const QString &to, const QStringList &listFiles);

public Q_SLOTS:

    void slotFileMetaDataChanged(QStringList fileList);

    void renamedFiles(const QString &from, const QString &to, const QStringList &listFiles);

};

#endif
