/**  Brief: Application Main Window
 *   Author: Caio Rodrigues - caiorss [at] rodrigues [at] gmail [dot] com
 *
 *
 ************************************************************/
#include <iostream>
#include <iomanip>
#include <functional>
#include <cassert>
#include <sstream>
#include <memory>

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>

#include "FormLoader.hpp"
#include "RecordTableModel.hpp"

namespace qtutils
{

    // Sender widgets: QPushButton, QListWidget, QCheckBox
    template<typename Sender, typename Callback>
    void on_clicked(Sender* pSender, Callback&& event_handler)
    {
        QObject::connect(pSender, &Sender::clicked, event_handler);
    }

    template<typename Sender, typename Receiver, typename Method>
    void on_clicked(Sender* pSender, Receiver* pReceiver, Method&& event_handler)
    {
        QObject::connect(pSender, &Sender::clicked, pReceiver, event_handler);
    }

    // Sender widgets: QListWidget
    template<typename Sender, typename Callback>
    void on_double_clicked(Sender* pSender, Callback&& event_handler)
    {
        QObject::connect(pSender, &Sender::doubleClicked, event_handler);
    }

    /** Creates a tray icon that toggles main window visiblity when clicked. */
    QSystemTrayIcon*
    make_window_toggle_trayicon(QMainWindow* wnd, QString icon_path, QString tooltip = "")
    {
        auto tray = new QSystemTrayIcon(wnd);
        auto appIcon = QIcon(icon_path);
        assert(!appIcon.isNull());

        wnd->setWindowIcon(appIcon);
        tray->setIcon(appIcon);
        tray->setToolTip(tooltip);
        tray->show();
        return tray;
    }
}

struct FileBookmarkItem
{
    QString uri_path;
    QString brief;
    QString description;

    FileBookmarkItem(){}
    FileBookmarkItem(QString uri_path, QString brief, QString description):
        uri_path(uri_path), brief(brief), description(description)
    {}
};

class FileBookmarkItemModel: public RecordTableModel<FileBookmarkItem>
{
public:

    FileBookmarkItemModel(){}

    explicit FileBookmarkItemModel(QWidget* parent)
        : RecordTableModel<FileBookmarkItem>(parent)
    {
    }

    // Constant
    int column_count() const override { return 3; }

    // Provide column name
    QString
    column_name(int column) const override
    {
        if(column == 0) { return "Type";     }
        if(column == 1) { return "File/URI"; }
        if(column == 2) { return "Path";     }
        return QString{};
    }

    // All columns are not editable by the user in the TableView, although
    // items can be modified by changing the model in the code.
    bool is_column_editable(int column) const override
    {
        return false;
    }

    /** Sets how all columns of a given item at a given row is displayed.
     */
    QString
    display_item_row(FileBookmarkItem const& item, int column) const override
    {
        // Check whether URI string is file or an URL, FTP ...
        auto is_uri_file = [](QString const& uri_str)
        {
            return not( uri_str.startsWith("http://")
                       || uri_str.startsWith("https://")
                       || uri_str.startsWith("ftp://"));
        };

#if 1
        QString file_name = item.uri_path;
        QString file_path;
        QString item_type = "url";

        if(is_uri_file(item.uri_path))
        {
            auto info = QFileInfo{item.uri_path};
            file_name = info.fileName();
            file_path = info.absolutePath();
            item_type = "file";
        }
        if(column == 0) return item_type;
        if(column == 1) return file_name;
        if(column == 2) return file_path;;
#endif

#if 0
        if(column == 0) return item.uri_path;
        if(column == 1) return item.brief;
        if(column == 2) return item.description;
#endif

        return QString("<EMPTY>");
    }

    bool
    set_element(int column, QVariant value, FileBookmarkItem& item) override
    {
        if(column == 0){
            item.uri_path = value.toString();
            return true;
        }
        if(column == 1){
            item.brief = value.toString();
            return true;
        }
        if(column == 2){
            item.description = value.toString();
            return true;
        }
        return false;
    }


};


class Tab_DesktopBookmarks
{
    FormLoader* loader;

    //======= Tab - Desktop Capture - Widgets =======//
    QWidget*               parent;
    QWidget*               tab_file_bookmarks;
    QTableView*            tview_disp;
    FileBookmarkItemModel* tview_model;
public:

    Tab_DesktopBookmarks(QWidget* parent, FormLoader* loader):
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

        auto mapper = new QDataWidgetMapper(parent);
        mapper->setModel(tview_model);
        mapper->addMapping(entry_ftype, 0, "text");
        mapper->addMapping(entry_fname, 1, "text");
        mapper->addMapping(entry_fpath, 2, "text");
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
            auto path = "file://" + QStandardPaths::standardLocations(p).at(0);
            QDesktopServices::openUrl(QUrl(path, QUrl::TolerantMode));
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

    // Disable copy-constructor and copy assignment operator
    Tab_DesktopBookmarks(Tab_DesktopBookmarks const& rhs) = delete;
    Tab_DesktopBookmarks& operator=(Tab_DesktopBookmarks const& rhs) = delete;

    void add_model_entry(QString uri_path, QString brief, QString description)
    {
        this->tview_model->add_item({uri_path, brief, description});
    }

    /// Open bookmark file in the Desktop Bookmark Tab
    void open_selected_bookmark_file()
    {

#if 1
        auto& self = *this;
        auto index = tview_disp->currentIndex();

        if(!index.isValid()) { return ; }

        auto item  = tview_model->at(index.row());

        auto file = item.uri_path;
        std::cout << " [INFO] Open file " << file.toStdString() << "\n";
        // Linux-only for a while

        auto file_uri_string = [&]
        {
            if(file.startsWith("http:") || file.startsWith("https:")
                ||  file.startsWith("ftp:") ||  file.startsWith("ftps:"))
                return file;
            return "file://" + file;
        }();
        QDesktopServices::openUrl(QUrl(file_uri_string, QUrl::TolerantMode));
#endif
    }

    void remove_selected_bookmark_file()
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

    void add_bookmark_file()
    {
        QString file = QFileDialog::getOpenFileName(parent, "Open File", ".");
        std::cout << " [INFO] Selected file = "
                  << file.toStdString() << std::endl;
        this->tview_model->add_item({file, "", ""});
        // self.save_settings();
    }


}; //----- End of class DesktopBookmarksTable ---------//


class AppMainWindow: public QMainWindow
{
private:
    FormLoader   loader;
    QWidget*     form;
    // Extract children widgets from from file
    QLineEdit*   cmd_input;
    QCheckBox*   chb_editable;
    QCheckBox*   chb_always_on_top;
    QListWidget* app_registry;


    //======== TrayIcon =============================//
    QSystemTrayIcon* tray_icon;


    std::unique_ptr<Tab_DesktopBookmarks> tab_deskbookmarks;

public:


    AppMainWindow()
        : loader{FormLoader(this, ":/assets/user_interface.ui")}
    {

        form = loader.GetForm();


        tab_deskbookmarks = std::make_unique<Tab_DesktopBookmarks>(this, &loader);

        //========= Tab - Application Launcher ==============///

        // Load controls named in the form "user_interface.ui"
        cmd_input         = loader.find_child<QLineEdit>("cmd_input");
        app_registry      = loader.find_child<QListWidget>("cmd_registry");
        chb_editable      = loader.find_child<QCheckBox>("chb_editable");
        chb_always_on_top = loader.find_child<QCheckBox>("chb_always_on_top");

        //========= Create Tray Icon =======================//

        // Do not quit when user clicks at close button
        this->setAttribute(Qt::WA_QuitOnClose, false);

        tray_icon = qtutils::make_window_toggle_trayicon(
            this,
            ":/assets/appicon.png"
            , "Tray Icon Test"
            );

        //========= Load Application state =================//

        this->setWindowAlwaysOnTop();
        this->load_settings();
        this->load_window_settings();

        // ========== Event Handlers of tray Icon ===============================//

        // Toggle this main window visible/hidden when user clicks at Tray Icon.
        QObject::connect(tray_icon, &QSystemTrayIcon::activated
                         , [&self = *this](QSystemTrayIcon::ActivationReason r)
                         {
                             // User clicked at QSystemTrayIcon
                             if(r  == QSystemTrayIcon::Trigger)
                             {
                                 if(!self.isVisible())
                                     self.show();
                                 else
                                     self.hide();
                                 return;
                             }
                             // std::cout << " [TRACE] TrayIcon Clicked OK." << std::endl;
                         });

        // ========== Set Event Handlers of Application Launcher Tab ============//

        // Enable Drag and Drop Event
        this->setAcceptDrops(true);

        // See: https://www.qtcentre.org/threads/15464-WindowStaysOnTopHint
        loader.on_clicked<QCheckBox>("chb_always_on_top",
                                     [&self = *this]
                                     {
                                #if 1
                                         QMessageBox::warning( &self
                                                              , "Error report"
                                                              , "Functionality not implemented yet."
                                                              );
                                #endif
                                // static auto flags = self.windowFlags();
                                // flags ^=  Qt::WindowStaysOnTopHint;
                                // self.show();
                                // self.activateWindow();
                                 });

        loader.on_button_clicked("btn_add", [&self = *this]
                                 {
                                     auto text = self.cmd_input->text();
                                     if(text.isEmpty()) { return; }
                                     auto item = new QListWidgetItem();
                                     item->setText(text);
                                     self.app_registry->insertItem(0, item);
                                     self.cmd_input->clear();
                                     self.save_settings();
                                 });
        // qtutils::on_clicked(btn_add,);

        // Signals and slots with member function pointer
        // QObject::connect(btn_remove, &QPushButton::clicked, this, &CustomerForm::Reset);

        // Signals and slots with lambda function
        loader.on_button_clicked("btn_run", [self = this]{ self->run_selected_item(); });


        // Launch application double clicked application from registry (QListWidget)
        qtutils::on_double_clicked(app_registry, [&self = *this]
                                   {
                                       if(self.chb_editable->isChecked())
                                       {
                                           auto item = self.app_registry->currentItem();
                                           if(item == nullptr) { return; }
                                           // Set item as editable
                                           item->setFlags( item->flags() | Qt::ItemIsEditable);
                                           self.save_settings();
                                           return;
                                       }
                                       self.run_selected_item();
                                       // auto command = items.first()->text();
                                   });

        loader.on_button_clicked("btn_remove",
                                 [&self = *this]
                                 {
                                     QListWidgetItem* pItem = self.app_registry->currentItem();
                                     if(pItem == nullptr) { return; }
                                     self.app_registry->removeItemWidget(pItem);
                                     delete pItem;
                                     self.save_settings();
                                 });


        // Save application state when the main Window is destroyed
        QObject::connect(this, &QMainWindow::destroyed, [this]
                         {
                             this->save_window_settings();
                             std::cout << " [INFO] Window closed Ok" << std::endl;
                         });


    } // --- End of CustomerForm ctor ------//

    // Run item selected in the QListWidget (ApplicationRegistry)
    void run_selected_item()
    {
        auto& self = *this;
        auto items = self.app_registry->selectedItems();
        if(items.isEmpty()) { return; }
        auto command = items.first()->text();

        bool status = QProcess::startDetached(command);

        std::cout << " [INFO] Run command " << command.toStdString()
                  << " status = " << (status ? "OK" : "FAILURE")
                  << std::endl;
    }

    // See: https://stackoverflow.com/questions/18934964
    bool eventFilter(QObject* object, QEvent* event) override
    {
        if(object == this->app_registry && event->type() == QEvent::KeyRelease)
        {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if(ke->key() == Qt::Key_Enter || Qt::Key_Return)
            {
                std::cout << " [INFO] User pressed Return on QListWidget " << std::endl;
            }
            return false;
        }
        return false;
    }

    /// Make this window stay alwys on top
    void setWindowAlwaysOnTop()
    {
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
    }

    QString
    get_settings_file()
    {
        // On Linux, the typical location of the setting file is:
        //   /home/<USER>/.config/<ApplicationName>.qconf
        QString settings_file = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).at(0)
                                + "/" + qApp->applicationName() + ".qconf";
        std::cout << " [INFO] Settings file = " << settings_file.toStdString() << std::endl;
        return settings_file;

    }

    void load_window_settings()
    {
        auto settings = QSettings("com.org.applauncher", "applauncherD");

        if(!settings.contains("window_pos"))
            return;

        if(!settings.contains("window_size"))
            return;

        auto pos = settings.value("window_pos").toPoint();
        this->move(pos.x(), pos.y());

        std::cout << " Position x = " << pos.x() << " ; y = " << pos.y() << std::endl;

        auto size = settings.value("window_size").toSize();

        std::cout << "Size w = " << size.width() << "; h = " << size.height() << std::endl;

        if(size.width() < 0 || size.height() < 0)
        {
            size.setWidth(400);
            size.setHeight(500);
        }

        this->resize(size);

        std::cout << " [TRACE] Window settings loaded OK." << std::endl;
    }

    void save_window_settings()
    {
        // First parameter is the Company name, second parameter is the
        // application name.
        auto settings = QSettings("com.org.applauncher", "applauncherD");
        settings.setValue("window_pos",  this->pos());
        settings.setValue("window_size", this->size());
        std::cout << " [TRACE] Window settings saved OK." << std::endl;
    }

    /// Load application state
    void load_settings()
    {
        QString settings_file = this->get_settings_file();

        // Abort if setting files does not exist
        if(!QFile(settings_file).exists()){ return; }

        auto settings = QSettings(settings_file, QSettings::IniFormat);
        auto commands = settings.value("commands/list").toStringList();
        for(auto const& cmd: commands){
            this->app_registry->addItem(cmd);
        }

        auto files_bookmarks = settings.value("files_bookmarks/list")
                                   .toStringList();
#if 1
        for(auto const& file: files_bookmarks){
            this->tab_deskbookmarks->add_model_entry(file, "", "");
        }
#endif

        std::cout << " [INFO] Settings loaded Ok." << std::endl;
    }

    /// Save application state
    void save_settings()
    {
#if 0
        auto settings_file = this->get_settings_file();
        auto settings = QSettings(settings_file, QSettings::IniFormat);
        QStringList list;
        for(int i = 0; i < this->app_registry->count(); i++)
        {
            QListWidgetItem* item = this->app_registry->item(i);
            list << item->text();
        }
        settings.setValue("commands/list", list);

        QStringList file_bookmarks;
        for(int i = 0; i < tview_model->count(); ++i)
        {
            file_bookmarks << tview_model->at(i).uri_path;
        }
        settings.setValue("files_bookmarks/list", file_bookmarks);
        settings.sync();
#endif
    }

    void dragEnterEvent(QDragEnterEvent* event) override
    {
        // if(event->source() != this->tab_file_bookmarks) return;

#if 0
        if(this->tab_file_bookmarks->isVisible())
        {
            const QMimeData* mimeData = event->mimeData();
            std::cout << "Drag Event" << std::endl;
            if(!mimeData->hasUrls())
                return;
            auto url = mimeData->urls()[0];
            QString path;

            if(url.isLocalFile())
                path = mimeData->urls()[0].toLocalFile();
            else
                path = mimeData->urls()[0].toString();

            std::cout << " [TRACE] Dragged file: " << path.toStdString() << "\n";
            // this->tview_disp->addItem(path);
            this->tview_model->add_item({path, "", ""});
            this->save_settings();
        }
#endif

    }

};


int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName("qapplauncher");   

    AppMainWindow maingui;
    maingui.setWindowIcon(QIcon(":/assets/appicon.png"));
    maingui.showNormal();


    return app.exec();
}

