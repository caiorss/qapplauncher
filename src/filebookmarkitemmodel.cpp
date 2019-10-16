#include "filebookmarkitemmodel.hpp"

FileBookmarkItemModel::FileBookmarkItemModel()
{
}

FileBookmarkItemModel::FileBookmarkItemModel(QWidget* parent)
    : qxstl::model::RecordTableModel<FileBookmarkItem>(parent)
{
}

// Constant => Returns the number of columns
int FileBookmarkItemModel::column_count() const
{
    return 4;
}

// Provide column name
QString
FileBookmarkItemModel::column_name(int column) const
{
    if(column == 0) { return "Type";     }
    if(column == 1) { return "File/URI"; }
    if(column == 2) { return "Path";     }
    if(column == 3) { return "Brief";    }
    return QString{};
}

// All columns are not editable by the user in the TableView, although
// items can be modified by changing the model in the code.
bool
FileBookmarkItemModel::is_column_editable(int column) const
{
    return false;
}

/** Sets how all columns of a given item at a given row is displayed.
     */
QString
FileBookmarkItemModel::display_item_row(FileBookmarkItem const& item, int column) const
{
    // Check whether URI string is file or an URL, FTP ...
    auto is_uri_file = [](QString const& uri_str)
    {
        return not( uri_str.startsWith("http://")
                   || uri_str.startsWith("https://")
                   || uri_str.startsWith("ftp://"));
    };

    QString file_name = item.uri_path;
    QString file_path;
    QString item_type = "URL";

    if(is_uri_file(item.uri_path))
    {
        auto info = QFileInfo{item.uri_path};
        file_name = info.fileName();
        file_path = info.absolutePath();

        if(QFileInfo(item.uri_path).isFile())
            item_type = "FILE";
        else
            item_type = "DIR";
    }
    if(column == 0) return item_type;
    if(column == 1) return file_name;
    if(column == 2) return file_path;
    if(column == 3) return item.brief;

    return QString("<EMPTY>");
}

bool
FileBookmarkItemModel::set_element(int column, QVariant value, FileBookmarkItem& item)
{
#if 0
    if(column == 0){
        item.uri_path = value.toString();
        return true;
    }
    if(column == 1){
        item.brief = value.toString();
        return true;
    }
#endif

    // Only the brief column is editable
    if(column == 3)
    {
        item.brief = value.toString();
        return true;
    }    

    return false;
}
