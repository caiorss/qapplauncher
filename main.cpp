#include <iostream>
#include <iomanip>
#include <functional>
#include <cassert>
#include <sstream>

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>

#include "FormLoader.hpp"

namespace qtutils
{

    // Sender widgets: QPushButton, QListWidget, QCheckBox
    template<typename Sender, typename Callback>
    void on_clicked(Sender* pSender, Callback&& event_handler)
    {
        QObject::connect(pSender, &Sender::clicked, event_handler);
    }

    // Sender widgets: QListWidget
    template<typename Sender, typename Callback>
    void on_double_clicked(Sender* pSender, Callback&& event_handler)
    {
        QObject::connect(pSender, &Sender::doubleClicked, event_handler);
    }

    QSystemTrayIcon*
    make_tray_icon(QMainWindow* wnd, QString icon_path, QString tooltip = "")
    {
        auto tray = new QSystemTrayIcon(wnd);
        tray->setToolTip(tooltip);
        auto appIcon = QIcon(icon_path);
        assert(!appIcon.isNull());
        wnd->setWindowIcon(appIcon);
        tray->show();
        return tray;
    }
}


class ApplicationLauncher: public FormLoader
{
private:
    QWidget*     form;
    // Extract children widgets from from file
    QLineEdit*   cmd_input;
    QPushButton* btn_add;
    QPushButton* btn_run;
    QPushButton* btn_remove;
    QCheckBox*   chb_editable;
    QCheckBox*   chb_always_on_top;
    QListWidget* cmd_registry;

    //======= Tab - Desktop Capture - Widgets =======//
    QWidget*     tab_file_bookmarks;
    QPushButton* btn_add_file;
    QPushButton* btn_open_file;
    QPushButton* btn_remove_file;
    QListWidget* tview_disp;

    //======== TrayIcon =============================//
    QSystemTrayIcon* tray_icon;

public:


    ApplicationLauncher()
        : FormLoader(":/assets/user_interface.ui")
    {
        form = this->FormLoader::GetForm();

        // Do not quit when user clicks at close button
        this->setAttribute(Qt::WA_QuitOnClose, false);

        //========= Tab - Application Launcher ==============///

        // Load controls named in the form "user_interface.ui"
        cmd_input    = form->findChild<QLineEdit*>("cmd_input");
        btn_add      = form->findChild<QPushButton*>("btn_add");
        btn_remove   = form->findChild<QPushButton*>("btn_remove");
        btn_run      = form->findChild<QPushButton*>("btn_run");
        cmd_registry = form->findChild<QListWidget*>("cmd_registry");
        chb_editable = form->findChild<QCheckBox*>("chb_editable");
        chb_always_on_top = form->findChild<QCheckBox*>("chb_always_on_top");

        //========= Tab - Desktop Capture =================//
        tab_file_bookmarks = form->findChild<QWidget*>("tab_file_bookmarks");
        btn_add_file = form->findChild<QPushButton*>("btn_add_file");
        btn_open_file = form->findChild<QPushButton*>("btn_open_file");
        btn_remove_file = form->findChild<QPushButton*>("btn_remove_file");

        tview_disp = form->findChild<QListWidget*>("tview_disp");
        assert(tview_disp != nullptr);

        //========= Create Tray Icon =======================//

        tray_icon = qtutils::make_tray_icon(this,
                                            ":/assets/appicon.png"
                                            , "Tray Icon Test");




        //========= Load Application state =================//

        // this->setWindowAlwaysOnTop();
        this->load_settings();

        // ========== Set Event Handlers =================//

        // Enable Drag and Drop Event
        this->setAcceptDrops(true);
        // tab_file_bookmarks->setAcceptDrops(true);
        // tview_disp->setAcceptDrops(true);
        tview_disp->setWhatsThis("List containing desktop file/directories bookmarks");

        btn_add_file->setAcceptDrops(true);

        // See: https://www.qtcentre.org/threads/15464-WindowStaysOnTopHint
        qtutils::on_clicked(chb_always_on_top,
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

        qtutils::on_clicked(btn_add,[&self = *this]
                            {
                             auto text = self.cmd_input->text();
                                if(text.isEmpty()) { return; }
                                auto item = new QListWidgetItem();
                                item->setText(text);
                                self.cmd_registry->insertItem(0, item);
                                self.cmd_input->clear();
                                self.save_settings();
                            });

        // Signals and slots with member function pointer
        // QObject::connect(btn_remove, &QPushButton::clicked, this, &CustomerForm::Reset);

        // Signals and slots with lambda function
        qtutils::on_clicked(btn_run, [self = this]{ self->run_selected_item(); });


        qtutils::on_double_clicked(cmd_registry, [&self = *this]
                                   {
                                       if(self.chb_editable->isChecked())
                                       {
                                           auto item = self.cmd_registry->currentItem();
                                           if(item == nullptr) { return; }
                                           // Set item as editable
                                           item->setFlags( item->flags() | Qt::ItemIsEditable);
                                           self.save_settings();
                                           return;
                                       }
                                       self.run_selected_item();
                                       // auto command = items.first()->text();
                                   });

        qtutils::on_clicked(btn_remove,
                            [&self = *this]
                            {
                                QListWidgetItem* pItem = self.cmd_registry->currentItem();
                                if(pItem == nullptr) { return; }
                                self.cmd_registry->removeItemWidget(pItem);
                                delete pItem;
                                self.save_settings();
                            });


        // Save application state when the main Window is destroyed
        QObject::connect(this, &QMainWindow::destroyed, []
                         {
                             std::cout << " [INFO] Window closed Ok" << std::endl;
                         });

        // =========== Event Handlers of Bookmark Table =========//

        qtutils::on_clicked(btn_add_file,
                            [&self = *this]
                            {
                                QString file = QFileDialog::getOpenFileName(
                                    &self, "Open File", ".");
                                std::cout << " [INFO] Selected file = "
                                          << file.toStdString() << std::endl;
                                self.tview_disp->addItem(file);
                                self.save_settings();
                            });

        auto open_selected_bookmark_file = [&self = *this]
        {
            QListWidgetItem* pItem= self.tview_disp->currentItem();
            // Abort on error
            if(!pItem){ return; }
            auto file = pItem->text();
            std::cout << " [INFO] Open file " << file.toStdString() << "\n";
            // Linux-only for a while
            QDesktopServices::openUrl(QUrl("file://" + file, QUrl::TolerantMode));
        };

        qtutils::on_clicked(btn_open_file, open_selected_bookmark_file);
        qtutils::on_double_clicked(tview_disp, open_selected_bookmark_file);

        qtutils::on_clicked(btn_remove_file,
                            [&self = *this]
                            {
                                QListWidgetItem* pItem = self.tview_disp->currentItem();
                                if(pItem == nullptr) { return; }
                                self.cmd_registry->removeItemWidget(pItem);
                                delete pItem;
                                self.save_settings();
                            });

    } // --- End of CustomerForm ctor ------//

    // Run item selected in the QListWidget
    void run_selected_item()
    {
        auto& self = *this;
        auto items = self.cmd_registry->selectedItems();
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
        if(object == this->cmd_registry && event->type() == QEvent::KeyRelease)
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

    /// Load application state
    void load_settings()
    {
        QString settings_file = this->get_settings_file();

        // Abort if setting files does not exist
        if(!QFile(settings_file).exists()){ return; }

        auto settings = QSettings(settings_file, QSettings::IniFormat);
        auto commands = settings.value("commands/list").toStringList();
        for(auto const& cmd: commands){
            this->cmd_registry->addItem(cmd);
        }

        auto files_bookmarks = settings.value("files_bookmarks/list")
                                   .toStringList();

        for(auto const& file: files_bookmarks){
            this->tview_disp->addItem(file);
        }

        std::cout << " [INFO] Settings loaded Ok." << std::endl;
    }

    /// Save application state
    void save_settings()
    {
        auto settings_file = this->get_settings_file();

        auto settings = QSettings(settings_file, QSettings::IniFormat);

        QStringList list;
        for(int i = 0; i < this->cmd_registry->count(); i++)
        {
            QListWidgetItem* item = this->cmd_registry->item(i);
            list << item->text();
        }
        settings.setValue("commands/list", list);

        QStringList file_bookmarks;
        for(int i = 0; i < this->tview_disp->count(); i++)
        {
            file_bookmarks << this->tview_disp->item(i)->text();
        }
        settings.setValue("files_bookmarks/list", file_bookmarks);

        settings.sync();
    }

    void dragEnterEvent(QDragEnterEvent* event) override
    {
        // if(event->source() != this->tab_file_bookmarks) return;

        if(this->tab_file_bookmarks->isVisible())
        {
            const QMimeData* mimeData = event->mimeData();
            std::cout << "Drag Event" << std::endl;
            if(!mimeData->hasUrls())
                return;
            QString path = mimeData->urls()[0].toLocalFile();
            std::cout << " [TRACE] Dragged file: " << path.toStdString() << "\n";
            this->tview_disp->addItem(path);
            this->save_settings();
        }

    }

};


int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName("qapplauncher");   

    ApplicationLauncher maingui;
    maingui.setWindowIcon(QIcon(":/assets/appicon.png"));
    maingui.showNormal();


    return app.exec();
}

