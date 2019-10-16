#ifndef TAB_DESKTOPBOOKMARKS_HPP
#define TAB_DESKTOPBOOKMARKS_HPP

#include <qxstl/FormLoader.hpp>
#include <qxstl/serialization.hpp>

#include "filebookmarkitemmodel.hpp"


#include <QtCore>
#include <QWidget>

namespace qxstl::serialization
{
template<>
inline QVariant value_writer(FileBookmarkItemModel& ref)
{
    QByteArray arr;
    QDataStream ss{&arr, QIODevice::WriteOnly};

    ss << ref.count();

    //QStringList list;
    for(int i = 0; i < ref.count(); ++i)
    {
        //list << ref.at(i).uri_path;
        auto& item = ref.at(i);
        ss << item.uri_path << item.brief << item.description;
    }
    //return list;
    return arr;
}

template<>
inline void value_reader(FileBookmarkItemModel& ref, QVariant value)
{
    QByteArray arr = value.toByteArray();
    QDataStream ss{&arr, QIODevice::ReadOnly};

    int count;
    ss >> count;

    for(int i = 0; i < count; i++)
    {
        QString uri_path, brief, description;
        ss >> uri_path >> brief >> description;
        ref.add_item(FileBookmarkItem{uri_path, brief, description});
    }
}

}

using qxstl::gui::FormLoader;

class Tab_DesktopBookmarks
{
    QWidget*               parent;
    FormLoader*            loader;
    QWidget*               tab_file_bookmarks;
    QTableView*            tview_disp;
    FileBookmarkItemModel* tview_model;
public:

    Tab_DesktopBookmarks(QWidget* parent, FormLoader* loader);

    // Disable copy-constructor and copy assignment operator
    Tab_DesktopBookmarks(Tab_DesktopBookmarks const& rhs) = delete;
    Tab_DesktopBookmarks& operator=(Tab_DesktopBookmarks const& rhs) = delete;

    void add_model_entry(QString uri_path, QString brief, QString description);

    // Returns true if this table is visible to the user
    bool is_visible();

    // Return item at Nth row of model
    FileBookmarkItem& at(int row);

    int count() const;

    /// Open bookmark file in the Desktop Bookmark Tab
    void open_selected_bookmark_file();

    void remove_selected_bookmark_file();

    void add_bookmark_file();


    template<typename Visitor>
    void accept(Visitor& visitor)
    {
        visitor.visit("tview_model", *tview_model);
    }

}; //----- End of class DesktopBookmarksTable ---------//



#endif // TAB_DESKTOPBOOKMARKS_HPP
