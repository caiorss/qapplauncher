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
    using column_index = int;
    using column_render_func = std::function<QString (TItem const&, column_index)>;

    RecordTableModel(  std::deque<QString> headers
                     , column_render_func render_function
                     )
        : m_headers{ std::move(headers)}
        , m_render_function{render_function}
    { }

    RecordTableModel(  QWidget* parent,
                       std::deque<QString> headers
                     , column_render_func render_function
                     )
        : QAbstractTableModel(parent)
        , m_headers{ std::move(headers)}
        , m_render_function{render_function}
    { }


    void add_item(TItem item)
    {
        int n = m_dataset.size();
        this->beginInsertRows(QModelIndex(), n, n);
        m_dataset.push_back(std::move(item));
        // Notify view that model data has changed => view updates itself
        this->endInsertRows();
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
            return m_headers[static_cast<size_t>(section)];
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
        return static_cast<int>(m_headers.size());
    }

    QVariant
    data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        if(role == Qt::DisplayRole)
        {
            auto& item = m_dataset.at( static_cast<size_t>(index.row()) );
            return m_render_function(item, index.column());
        }
        // FIXME: Implement me!
        return QVariant();
    }

private:
    const std::deque<QString> m_headers;
    std::deque<TItem>         m_dataset;
    column_render_func         m_render_function;

}; //---- End of class RecordTableModel ---//



#endif // RECORDTABLEMODEL_HPP
