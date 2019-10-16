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

namespace qxstl::model
{

/**
 *
 *  Note: References that helped implementing this class.
 *
 *
 *   + How to prevent QTableView item from getting cleared on double-click
 *     - https://stackoverflow.com/questions/28081988/
 *
 *   + Address Book Example
 *    - https://doc.qt.io/archives/4.6/itemviews-addressbook.html
 *
 *   + Mapping Data Into Widgets
 *    - http://www.crossplatform.ru/node/314
 *
 *   + Simple Widget Mapper Example
 *    - https://doc.qt.io/archives/4.6/itemviews-simplewidgetmapper.html
 *
 *   + How to make a constant correct read only model view architecture in QT?
 *    - https://stackoverflow.com/questions/9988342/
 *
 **************************************************************************/
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
    virtual QString column_name(int column) const = 0;

    // Derived classes must override this member function
    virtual QString display_item_row(TItem const& item, int column) const = 0;

    // Implementation must decide how to set model's columns.
    virtual bool set_element(int column, QVariant value, TItem& item) = 0;

    // Implementation must decide which columns are editable or not in the view
    // If the column 0 is editable, this function returns true for this column,
    // if the column 1 is not editable, this function returns false for column 1.
    virtual bool is_column_editable(int column) const = 0;

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

        if(role == Qt::DisplayRole || role == Qt::EditRole)
        {
            auto& item = m_dataset.at( static_cast<size_t>(index.row()) );
            return  this->display_item_row(item, index.column());
        }
        // FIXME: Implement me!
        return QVariant();
    }

    /** QT Docs: The base class implementation returns a combination of flags that
     *  enables the item (ItemIsEnabled) and allows it to be selected (ItemIsSelectable).
     ***************************************************************************/
    Qt::ItemFlags
    flags(const QModelIndex &index) const override
    {
        // return  Qt::ItemIsEnabled;
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        // Make column read-only by the user if it is not editable.
        if(!this->is_column_editable(index.column()))
            return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;

        // All columns are editable
        return QAbstractTableModel::flags(index)
               | Qt::ItemIsEnabled | Qt::ItemIsSelectable
               | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
        // return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsSelectable;
    }

    /**  Inserts item in the table model, item by item, not row by row
     *
     *  @param index - pair row, column of current element being edited.
     *  @param value - New value of the element being edited
     *  @param role  - Sets the type of operation on the table item
     *
     * QT Docs:
     *  The dataChanged() signal should be emitted if the data was successfully set.
     *  The base class implementation returns false. This function and data()
     *  must be reimplemented for editable models.
     ***********************************************************************/
    bool
    setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if(!index.isValid() && role != Qt::EditRole) { return false; }
        int row = index.row();
        int col = index.column();
        TItem& item = m_dataset[row];
        if(set_element(col, value, item))
        {
            emit this->dataChanged(index, index);
            return true;
        }
        return false;
    }


private:
    std::deque<TItem>         m_dataset;

}; //---- End of class RecordTableModel ---//


}

#endif // RECORDTABLEMODEL_HPP
