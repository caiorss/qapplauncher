/*  Brief:  Generic templated model for QTableWidget classes
 *  Author: Caio Rodrigues - caiorss [dot] rodrigues [at] gmail [dot] com
 *
 *
 ************************************************************************/

#ifndef RECORDTABLEMODEL_HPP
#define RECORDTABLEMODEL_HPP

#include <iostream>
#include <functional>
#include <deque>

#include <QtWidgets>
#include <QApplication>
#include <QSysInfo>

template<typename TItem>
class RecordTableModel: public QAbstractTableModel
{
public:

    RecordTableModel()
    { }

    RecordTableModel(QWidget* parent): QAbstractTableModel(parent)
    { }

    virtual ~RecordTableModel() = default;

    // Number of columns
    virtual int column_count() const = 0;

    // Get the column name given its index
    virtual QString column_name(int column_index) const = 0;

    // Derived classes must override this member function
    virtual QString display_item_row(TItem const& item, int column_index) const = 0;

    void add_item(TItem item)
    {
        int n = m_dataset.size();
        this->beginInsertRows(QModelIndex(), n, n);
        m_dataset.push_back(std::move(item));
        // Notify view that model data has changed => view updates itself
        this->endInsertRows();
    }

    /// Remove item N or row N
    void remove_item(int n)
    {
        if(n > m_dataset.size()){ return; }
        this->beginRemoveRows(QModelIndex(), n, n);
        m_dataset.erase(m_dataset.begin() + n);
        this->endRemoveRows();
    }

    int count() const
    {
        return m_dataset.size();
    }

    TItem& at(int n)
    {
        return m_dataset.at(n);
    }        

    // Return begin iterator
    auto begin() { return m_dataset.begin(); }
    auto end()   { return m_dataset.end();   }

    auto begin() const { return m_dataset.begin(); }
    auto end()   const  { return m_dataset.end();   }


    void clear()
    {
        int n = static_cast<int>(m_dataset.size());
        this->beginRemoveRows(QModelIndex(), 0, n - 1);
        m_dataset.clear();
        this->endRemoveRows();
    }

    //========= Necessary Member Functions for Redendering the Model =======//

    // Header:
    QVariant
    headerData(  int section
               , Qt::Orientation orientation
               , int role = Qt::DisplayRole ) const override
    {
        if(role != Qt::DisplayRole) { return QVariant{}; }
        if(orientation == Qt::Orientation::Horizontal)
        {
            // return m_headers[static_cast<size_t>(section)];
            return this->column_name(section);
        }
        return QString::number(section);
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return static_cast<int>(m_dataset.size());
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return this->column_count();
    }

    QVariant
    data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            auto& item = m_dataset.at( static_cast<size_t>(index.row()) );
            return  this->display_item_row(item, index.column());
        }
        // FIXME: Implement me!
        return QVariant();
    }

private:
    std::deque<TItem>         m_dataset;   

}; //---- End of class RecordTableModel ---//



#endif // RECORDTABLEMODEL_HPP
