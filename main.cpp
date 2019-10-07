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

class CustomerForm: public FormLoader
{
private:
    QWidget*     form;
    // Extract children widgets from from file
    QLineEdit*   cmd_input;
    QPushButton* btn_add;
    QPushButton* btn_run;
    QPushButton* btn_remove;
    QListWidget* cmd_registry;
public:


    CustomerForm()
        : FormLoader(":/assets/user_interface.ui")
    {
        form = this->FormLoader::GetForm();
        // Load controls named in the form "user_interface.ui"
        cmd_input  = form->findChild<QLineEdit*>("cmd_input");
        btn_add    = form->findChild<QPushButton*>("btn_add");
        btn_remove = form->findChild<QPushButton*>("btn_remove");
        btn_run    = form->findChild<QPushButton*>("btn_run");
        cmd_registry = form->findChild<QListWidget*>("cmd_registry");


        // ========== Set Event Handlers =================//

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
                         });

        // Signals and slots with member function pointer
        // QObject::connect(btn_remove, &QPushButton::clicked, this, &CustomerForm::Reset);

        // Signals and slots with lambda function
        QObject::connect(btn_run, &QPushButton::clicked,
                         [&self = *this]
                         {
                             auto items = self.cmd_registry->selectedItems();
                             if(items.isEmpty()) { return; }
                             auto command = items.first()->text();

                             bool status = QProcess::startDetached(command);

                             std::cout << " [INFO] Run command " << command.toStdString()
                                       << " status = " << (status ? "OK" : "FAILURE")
                                       << std::endl;
                         });


        QObject::connect(btn_remove, &QPushButton::clicked,
                         [&self = *this]
                         {
                             QListWidgetItem* pItem = self.cmd_registry->currentItem();
                             if(pItem == nullptr) { return; }
                             self.cmd_registry->removeItemWidget(pItem);
                             delete pItem;
                         });


        // Save application state when the main Window is destroyed
        QObject::connect(this, &QMainWindow::destroyed, []{
            std::cout << " [INFO] Window closed Ok" << std::endl;
        });                

    } // --- End of CustomerForm ctor ------//


};


int main(int argc, char** argv)
{
    std::cout << " [INFO] Starting Application" << std::endl;

    QApplication app(argc, argv);
    app.setApplicationName("qapplauncher");

    std::cout << " [INFO] Starting Application" << std::endl;

    // Useful for debugging
    std::cout << " [INFO] QTVersion = " << ::qVersion() << std::endl;


    CustomerForm form;
    form.setWindowIcon(QIcon(":/images/appicon.png"));
    form.showNormal();


    return app.exec();
}

