#ifndef FACTORIES_H
#define FACTORIES_H

#include <QAbstractListModel>
#include <QtXml/QDomDocument>
#include "StationFactory.h"


namespace Tide {

class Factories;

class FactoryProxy: public ClientProxy {
public:
    FactoryProxy(Factories* parent, const QString& key);
    void whenFinished(const Status&);
    ClientProxy* clone();

private:

    Factories* m_Parent;
    QString m_Key;
};


class Factories: public QAbstractListModel
{
    Q_OBJECT

public:

    enum FactoryRoles {
        NameRole = Qt::UserRole + 1,
        AboutRole,
        HomePageRole
    };


public:

    // create new factory info model
    Factories(const QList<StationFactory*>& factories, QObject* parent = 0);

    //! Reimplemented from QAbstractItemModel
    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    //! Reimplemented from QAbstractItemModel
    QHash<int, QByteArray> roleNames() const;


    StationFactory* instance(int row);
    StationFactory* instance(const QString& key);

    ~Factories();

    Q_INVOKABLE void update(int row);



signals:

    void availableChanged(const QString&);


private:

    QList<StationFactory*> m_Factories;
    QHash<QString, QDomDocument> m_Info;

};



}

#endif // FACTORIES_H
