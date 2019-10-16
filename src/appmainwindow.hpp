#ifndef APPMAINWINDOW_HPP
#define APPMAINWINDOW_HPP

#include <QtWidgets>

//------ Headers of helper classes and namespaces -------//
#include <qxstl/event.hpp>
#include <qxstl/FormLoader.hpp>
#include <qxstl/RecordTableModel.hpp>

// ----- Headers of Domain Classes -----//
#include "filebookmarkitemmodel.hpp"
#include "FileBookmarkItem.hpp"
#include "tab_applicationlauncher.hpp"
#include "tab_desktopbookmarks.hpp"


class AppMainWindow: public QMainWindow
{
private:
    FormLoader   loader;
    QWidget*     form;

    //======== TrayIcon =============================//
    QSystemTrayIcon* tray_icon;

    std::unique_ptr<Tab_DesktopBookmarks>    tab_deskbookmarks;
    std::unique_ptr<Tab_ApplicationLauncher> tab_applauncher;

public:


    AppMainWindow();
    /// Make this window stay alwys on top
    void setWindowAlwaysOnTop();

    QString get_settings_file();

    void load_window_settings();

    void save_window_settings();

    /// Load application state
    void load_settings();

    /// Save application state
    void save_settings();

    void dragEnterEvent(QDragEnterEvent* event) override;

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

};



#endif // APPMAINWINDOW_HPP
