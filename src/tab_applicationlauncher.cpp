#include <iostream>

#include "tab_applicationlauncher.hpp"
#include <qxstl/event.hpp>

namespace qx = qxstl::event;

Tab_ApplicationLauncher::Tab_ApplicationLauncher(
      QWidget* parent
    , FormLoader* loader
    , std::function<void ()> callback
    ): parent(parent), loader{loader}, save_settings_callback{callback}
{
    //========= Tab - Application Launcher ==============///

    // Load controls named in the form "user_interface.ui"
    cmd_input         = loader->find_child<QComboBox>("cmd_input");
    app_registry      = loader->find_child<QListWidget>("cmd_registry");
    chb_editable      = loader->find_child<QCheckBox>("chb_editable");
    chb_always_on_top = loader->find_child<QCheckBox>("chb_always_on_top");

    // Combobox and list view share the same model
    cmd_input->setModel(app_registry->model());



    // See: https://www.qtcentre.org/threads/15464-WindowStaysOnTopHint
    loader->on_clicked<QCheckBox>("chb_always_on_top",
                                  [&self = *this]
                                  {
#if 1
                                      QMessageBox::warning( self.parent
                                                           , "Error report"
                                                           , "Functionality not implemented yet."
                                                           );
#endif
                                      // static auto flags = self.windowFlags();
                                      // flags ^=  Qt::WindowStaysOnTopHint;
                                      // self.show();
                                      // self.activateWindow();
                                  });

    loader->on_button_clicked("btn_add", [&self = *this]
                              {
                                  auto text = self.cmd_input->currentText();
                                  if(text.isEmpty()) { return; }
                                  auto item = new QListWidgetItem();
                                  item->setText(text);
                                  // self.app_registry->insertItem(0, item);
                                  self.app_registry->addItem(item);
                                  // self.cmd_input->clear();
                                  self.save_settings_callback();
                                  //self.save_settings();
                              });
    // qtutils::on_clicked(btn_add,);

    // Signals and slots with member function pointer
    // QObject::connect(btn_remove, &QPushButton::clicked, this, &CustomerForm::Reset);

    // Signals and slots with lambda function
    // loader->on_button_clicked("btn_run", [self = this]{ self->run_selected_item(); });
    loader->on_button_clicked("btn_run", this
                              , &Tab_ApplicationLauncher::run_combobox_command) ;

    // qx::set_shortcut(cmd_input, Qt::Key_Return, std::bind(&Tab_ApplicationLauncher::run_combobox_command, this));

    // Launch application double clicked application from registry (QListWidget)
    loader->on_double_clicked<QListWidget>("cmd_registry", [&self = *this]
                               {
                                   if(self.chb_editable->isChecked())
                                   {
                                       auto item = self.app_registry->currentItem();
                                       if(item == nullptr) { return; }
                                       // Set item as editable
                                       item->setFlags( item->flags() | Qt::ItemIsEditable);
                                       self.save_settings_callback();
                                       return;
                                   }
                                   self.run_selected_item();
                                   // auto command = items.first()->text();
                               });

    loader->on_button_clicked("btn_remove",
                              [&self = *this]
                              {
                                  QListWidgetItem* pItem = self.app_registry->currentItem();
                                  if(pItem == nullptr) { return; }
                                  self.app_registry->removeItemWidget(pItem);
                                  delete pItem;
                                  self.save_settings_callback();
                              });

} // --- End of Tab_ApplicationLauncher CTOR -------//

void Tab_ApplicationLauncher::run_selected_item()
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

void Tab_ApplicationLauncher::run_combobox_command()
{
    auto command = cmd_input->currentText();
    bool status = QProcess::startDetached(command);
    std::cout << " [INFO] Run command " << command.toStdString()
              << " status = " << (status ? "OK" : "FAILURE")
              << std::endl;
}

void  Tab_ApplicationLauncher::add_item(QString command)
{
    this->app_registry->addItem(command);
}

int Tab_ApplicationLauncher::count()
{
    return this->app_registry->count();
}

/// Return pointer to element at nth row
QListWidgetItem*
Tab_ApplicationLauncher::at(int row)
{
    return this->app_registry->item(row);
}

