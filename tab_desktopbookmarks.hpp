#ifndef TAB_DESKTOPBOOKMARKS_HPP
#define TAB_DESKTOPBOOKMARKS_HPP

#include "FormLoader.hpp"
#include "filebookmarkitemmodel.hpp"
#include <QtCore>
#include <QWidget>

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

}; //----- End of class DesktopBookmarksTable ---------//



#endif // TAB_DESKTOPBOOKMARKS_HPP
