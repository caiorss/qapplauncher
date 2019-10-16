#include <qxstl/serialization.hpp>
#include "appmainwindow.hpp"

namespace qx = qxstl::event;

AppMainWindow::AppMainWindow()
    : loader{FormLoader(this, ":/assets/user_interface.ui")}
{
    form = loader.GetForm();

    //====== Set Up Tabs ===================================/q
    tab_applauncher   = std::make_unique<Tab_ApplicationLauncher>(
        this,
        &loader,
        std::bind(&AppMainWindow::save_settings, this)
        );
    tab_deskbookmarks = std::make_unique<Tab_DesktopBookmarks>(this, &loader);

    //===== Set up User Interface Theme =================//

    qx::set_app_dark_style();

    //========= Create Tray Icon =======================//

    // Do not quit when user clicks at close button
    this->setAttribute(Qt::WA_QuitOnClose, false);

    tray_icon = qx::make_window_toggle_trayicon(
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

    // Register pointer to static member function
    loader.on_button_clicked("btn_quit_app",
                             [self = this]
                             {
                                 self->save_settings();
                                 QApplication::quit();
                             });

    loader.on_button_clicked("btn_show_help", &QWhatsThis::enterWhatsThisMode);

    loader.on_src_clicked<QCheckBox>("chb_dark_theme", [](QCheckBox* sender)
                                     {
                                         if(sender->isChecked())
                                             qx::set_app_dark_style();
                                         else
                                             qx::set_app_default_style();
                                     });

    loader.on_button_clicked(
        "btn_install_icon",
        []{
            QString desktop_path =
                QStandardPaths::standardLocations(QStandardPaths::DesktopLocation)[0];

            qx::create_linux_desktop_shortcut(
                desktop_path
                , ":/assets/appicon.png"
                , "Application for bookmarking files, directories and applications"
                );
        });

    // Save application state when the main Window is destroyed
    QObject::connect(this, &QMainWindow::destroyed, [this]
                     {
                         this->save_window_settings();
                         std::cout << " [INFO] Window closed Ok" << std::endl;
                     });


} // --- End of CustomerForm ctor ------//

#if 0
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
#endif

/// Make this window stay alwys on top
void
AppMainWindow::setWindowAlwaysOnTop()
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

QString
AppMainWindow::get_settings_file()
{
    // On Linux, the typical location of the setting file is:
    //   /home/<USER>/.config/<ApplicationName>.qconf
    QString settings_file = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).at(0)
                            + "/" + qApp->applicationName() + ".qconf";
    std::cout << " [INFO] Settings file = " << settings_file.toStdString() << std::endl;
    return settings_file;

}

void
AppMainWindow::load_window_settings()
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

void
AppMainWindow::save_window_settings()
{
    // First parameter is the Company name, second parameter is the
    // application name.
    auto settings = QSettings("com.org.applauncher", "applauncherD");
    settings.setValue("window_pos",  this->pos());
    settings.setValue("window_size", this->size());
    std::cout << " [TRACE] Window settings saved OK." << std::endl;
}

/// Load application state
void
AppMainWindow::load_settings()
{
    QString settings_file = this->get_settings_file();    
    // Abort if setting files does not exist
    if(!QFile(settings_file).exists()){ return; }

    qxstl::serialization::FileReader reader(settings_file);
    reader(*tab_applauncher);
    reader(*tab_deskbookmarks);

    std::cout << " [INFO] Settings loaded Ok." << std::endl;
}

/// Save application state
void AppMainWindow::save_settings()
{

    std::cout << " [INFO] START Settings saved OK" << std::endl;

    auto settings_file = this->get_settings_file();

    qxstl::serialization::FileWriter writer(settings_file);
    writer(*tab_applauncher);
    writer(*tab_deskbookmarks);
    std::cout << " [INFO] END Settings saved OK" << std::endl;
}


void
AppMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
#if 1
    if(this->tab_deskbookmarks->is_visible())
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
        this->tab_deskbookmarks->add_model_entry(path, "", "");
        this->save_settings();
    }
#endif

}

