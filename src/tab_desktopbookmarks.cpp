#include "tab_desktopbookmarks.hpp"


Tab_DesktopBookmarks::Tab_DesktopBookmarks(QWidget* parent, FormLoader* loader):
    parent(parent), loader{loader}
{
    //========= Tab - File Bookmark =================//

    tab_file_bookmarks = loader->find_child<QWidget>("tab_file_bookmarks");

    tview_disp = loader->find_child<QTableView>("tview_disp");
    tview_disp->horizontalHeader()->setStretchLastSection(true);
    tview_disp->verticalHeader()->hide();
    tview_disp->setSelectionMode(QTableView::SingleSelection);
    tview_disp->setSelectionBehavior(QTableView::SelectRows);
    tview_disp->setDragDropMode(QTableView::InternalMove);
    tview_disp->setShowGrid(false);
    tview_disp->setSortingEnabled(true);
    tview_disp->setWhatsThis("List containing desktop file/directories bookmarks");
    tview_disp->setFocus();

    tview_model = new FileBookmarkItemModel(parent);
    tview_disp->setModel(tview_model);

    // Only works after the model is set
    // Hide path column
    tview_disp->setColumnHidden(2, true);

    auto entry_ftype = loader->find_child<QLineEdit>("entry_file_type");
    entry_ftype->setReadOnly(true);
    auto entry_fname = loader->find_child<QLineEdit>("entry_file_name");
    entry_fname->setReadOnly(true);
    auto entry_fpath = loader->find_child<QLineEdit>("entry_file_path");
    entry_fpath->setReadOnly(true);
    auto entry_brief = loader->find_child<QLineEdit>("entry_file_brief");
    // entry_brief->setReadOnly(true);



    auto mapper = new QDataWidgetMapper(parent);
    mapper->setModel(tview_model);
    mapper->addMapping(entry_ftype, 0, "text");
    mapper->addMapping(entry_fname, 1, "text");
    mapper->addMapping(entry_fpath, 2, "text");
    mapper->addMapping(entry_brief, 3, "text");
    mapper->toFirst();

    // Event triggered when the selection of current row is changed.
    QObject::connect(tview_disp->selectionModel(),
                     &QItemSelectionModel::currentRowChanged,
                     [=](QModelIndex i1, QModelIndex i2)
                     {
                         std::cout << " [TRACE] Selection changed to index = "
                                   << i1.row() << std::endl;
                         mapper->setCurrentModelIndex(i1);
                     });

    // Update all widgets whenever a new selection iof QTableView changes

    // =========== Event Handlers of Bookmark Table =========//

    loader->on_button_clicked( "btn_add_file",
                              std::bind(&Tab_DesktopBookmarks::add_bookmark_file, this));


    loader->on_button_clicked( "btn_open_file",
                              std::bind(&Tab_DesktopBookmarks::open_selected_bookmark_file, this));

    // qtutils::on_double_clicked(tview_disp, open_selected_bookmark_file);
#if 1
    loader->on_double_clicked<QTableView>( "tview_disp",
                                          [this]{
                                              this->open_selected_bookmark_file();
                                          });
#endif

    loader->on_button_clicked(
        "btn_remove_file",
        std::bind(&Tab_DesktopBookmarks::remove_selected_bookmark_file, this));



    //================= Uitility Buttons =========================//

    auto open_stdpath = [](QStandardPaths::StandardLocation p)
    {
        auto url = QUrl::fromLocalFile(QStandardPaths::standardLocations(p).at(0));
        QDesktopServices::openUrl(url);
    };

    loader->on_button_clicked("btn_open_home",
                              std::bind(open_stdpath, QStandardPaths::HomeLocation));

    loader->on_button_clicked("btn_open_docs",
                              std::bind(open_stdpath, QStandardPaths::DocumentsLocation));

    loader->on_button_clicked("btn_open_desktop",
                              std::bind(open_stdpath, QStandardPaths::DesktopLocation));

    loader->on_button_clicked("btn_open_fonts",
                              std::bind(open_stdpath, QStandardPaths::FontsLocation));

} //----- End of Tab_DesktopBookmarks constructor ----------//


void Tab_DesktopBookmarks::add_model_entry(QString uri_path, QString brief, QString description)
{
    this->tview_model->add_item({uri_path, brief, description});
}

// Returns true if this table is visible to the user
bool Tab_DesktopBookmarks::is_visible()
{
    return this->tab_file_bookmarks->isVisible();
}

// Return item at Nth row of model
FileBookmarkItem&
Tab_DesktopBookmarks::at(int row)
{
    return this->tview_model->at(row);
}

int Tab_DesktopBookmarks::count() const
{
    return this->tview_model->count();
}

/// Open bookmark file in the Desktop Bookmark Tab
void Tab_DesktopBookmarks::open_selected_bookmark_file()
{
    auto& self = *this;
    auto index = tview_disp->currentIndex();

    if(!index.isValid()) { return ; }

    auto item  = tview_model->at(index.row());

    auto file = item.uri_path;
    std::cout << " [INFO] Open file " << file.toStdString() << "\n";
    // Linux-only for a while

    auto url = [&]
    {
        if(file.startsWith("http:") || file.startsWith("https:")
            ||  file.startsWith("ftp:") ||  file.startsWith("ftps:"))
            return QUrl(file, QUrl::TolerantMode);
        return QUrl::fromLocalFile(file);
    }();
    QDesktopServices::openUrl(url);
}

void Tab_DesktopBookmarks::remove_selected_bookmark_file()
{

    // QSTL_WARNING_FUNCTION_NOT_IMPLEMENTED();
    auto& self = *this;
    auto index = self.tview_disp->currentIndex();
    // Abort on error
    if(!index.isValid()) { return; }
    // QListWidgetItem* pItem = self.tview_disp->currentItem();
    self.tview_model->remove_item(index.row());

    // self.save_settings();
}

void Tab_DesktopBookmarks::add_bookmark_file()
{
    QString file = QFileDialog::getOpenFileName(parent, "Open File", ".");
    std::cout << " [INFO] Selected file = "
              << file.toStdString() << std::endl;
    this->tview_model->add_item({file, "", ""});
    // self.save_settings();
}
