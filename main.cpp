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
    QPushButton* btn_add_file;
    QPushButton* btn_open_file;
    QListWidget* tview_disp;
public:


    ApplicationLauncher()
        : FormLoader(":/assets/user_interface.ui")
    {
        form = this->FormLoader::GetForm();

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

        btn_add_file = form->findChild<QPushButton*>("btn_add_file");
        btn_open_file = form->findChild<QPushButton*>("btn_open_file");

        tview_disp = form->findChild<QListWidget*>("tview_disp");
        assert(tview_disp != nullptr);

        // this->setWindowAlwaysOnTop();
        this->load_settings();

        // ========== Set Event Handlers =================//


        // See: https://www.qtcentre.org/threads/15464-WindowStaysOnTopHint
        QObject::connect(chb_always_on_top, &QCheckBox::clicked,
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


        // Signals and slots with lambda function
        QObject::connect(btn_add, &QPushButton::clicked,
                         [&self = *this]
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
        QObject::connect(btn_run, &QPushButton::clicked
                         ,[&self = *this]{
                             self.run_selected_item();
                         });

        QObject::connect(cmd_registry, &QListWidget::doubleClicked
                         ,[&self = *this]
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

             //this , &ApplicationLauncher::run_selected_item);


        QObject::connect(btn_remove, &QPushButton::clicked,
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

        QObject::connect(btn_add_file, &QPushButton::clicked,
                         [&self = *this]
                         {
                             QString file = QFileDialog::getOpenFileName(
                                 &self, "Open File", ".");
                             std::cout << " [INFO] Selected file = "
                                       << file.toStdString() << std::endl;
                             self.tview_disp->addItem(file);
                         });

        QObject::connect(btn_open_file, &QPushButton::clicked,
                         [&self = *this]
                         {
                             QListWidgetItem* pItem= self.tview_disp->currentItem();
                             // Abort on error
                             if(!pItem){ return; }
                             auto file = pItem->text();
                             std::cout << " [INFO] Open file " << file.toStdString() << "\n";
                             auto args = QList<QString>{file};
                             // Linux-only for a while
                             bool status = QProcess::startDetached("xdg-open", args);
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
        std::cout << " [INFO] Settings loaded Ok." << std::endl;
    }

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
        settings.sync();
    }


};


int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName("qapplauncher");

    ApplicationLauncher form;
    form.setWindowIcon(QIcon(":/images/appicon.png"));
    form.showNormal();


    return app.exec();
}

