#ifndef FILEBOOKMARKITEMMODEL_HPP
#define FILEBOOKMARKITEMMODEL_HPP

#include "FileBookmarkItem.hpp"
#include <qxstl/RecordTableModel.hpp>

class FileBookmarkItemModel: public qxstl::model::RecordTableModel<FileBookmarkItem>
{
public:

    FileBookmarkItemModel();

    explicit FileBookmarkItemModel(QWidget* parent);

    // Constant
    int column_count() const override;

    // Provide column name
    QString column_name(int column) const override;

    // All columns are not editable by the user in the TableView, although
    // items can be modified by changing the model in the code.
    bool is_column_editable(int column) const override;

    /** Sets how all columns of a given item at a given row is displayed.
     */
    QString display_item_row(FileBookmarkItem const& item, int column) const override;

    bool set_element(int column, QVariant value, FileBookmarkItem& item);
};



#endif // FILEBOOKMARKITEMMODEL_HPP
