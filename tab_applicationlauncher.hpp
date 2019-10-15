#ifndef TAB_APPLICATIONLAUNCHER_HPP
#define TAB_APPLICATIONLAUNCHER_HPP

#include "FormLoader.hpp"


class Tab_ApplicationLauncher
{
    FormLoader* loader;
    QWidget*    parent;

    // Extract children widgets from from file
    QComboBox*   cmd_input;
    QCheckBox*   chb_editable;
    QCheckBox*   chb_always_on_top;
    QListWidget* app_registry;

    std::function<void ()> save_settings_callback;
public:

    Tab_ApplicationLauncher(QWidget* parent, FormLoader* loader,
                            std::function<void ()> save_settings_callback);

    /// Run item selected in the QListWidget (ApplicationRegistry)
    void run_selected_item();

    /// Run command entered by user in combobox
    void run_combobox_command();

    /// Add new command to command registry widget
    void add_item(QString command);

    /// Return number of elements in the command registry list widget
    int count();

    /// Return pointer to element at nth row
    QListWidgetItem*
    at(int row);

    void save_settings();


}; // ---- End of class Tab_ApplicationLauncher ----------//



#endif // TAB_APPLICATIONLAUNCHER_HPP
