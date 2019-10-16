#ifndef QTUTILS_HPP
#define QTUTILS_HPP

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>

#include <string>
#include <cassert>

namespace qxstl::event
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

/** Set shortcut to widget */
template<typename Callback>
QShortcut*
set_shortcut(QWidget*  widget, QKeySequence keybind, Callback&&  event_handler)
{
    auto shortcut = new QShortcut(keybind, widget);
    QObject::connect(shortcut, &QShortcut::activated,
                     [=]{
                         // Get pointer to current application, same as qApp
                         auto app = static_cast<QApplication *>(QCoreApplication::instance());
                         // Get widget that has focus
                         QWidget* focus_widget = app->focusWidget();
                         // If the target widget has focus, call event handler (callback)
                         if(focus_widget == widget) { event_handler(); }
                     });
    return  shortcut;
}

inline void set_app_style_sheet(QString file_name)
{
    using namespace std::string_literals;
    auto app = static_cast<QApplication *>(QCoreApplication::instance());
    QFile file(file_name);
    if(!file.open(QFile::ReadOnly))
    {
        throw std::runtime_error(" ERROR: style sheet file not found <"s
                                 + file_name.toStdString() + "> ");
    }
   QString styleSheet = QLatin1String(file.readAll());
   app->setStyleSheet(styleSheet);
}

/** Set global application style to Dark Theme
  * See: https://gist.github.com/QuantumCD/6245215
  */
inline void set_app_dark_style()
{
    auto app = static_cast<QApplication *>(QCoreApplication::instance());
    app->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app->setPalette(darkPalette);
    app->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

/** @brief Reset application style. */
inline void set_app_default_style()
{
    auto app = static_cast<QApplication *>(QCoreApplication::instance());
    app->setPalette(qApp->style()->standardPalette());
    app->setStyleSheet("");
}

/** Creates a tray icon that toggles main window visiblity when clicked. */
inline QSystemTrayIcon*
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

inline
bool create_linux_desktop_shortcut(
    QString const& destDirectory,
    QString const& icon_file,
    QString const& comment = ""
    )
{
    QString imagePath = QCoreApplication::applicationDirPath() + "/icon.png";
    // Extract resource file to disk to the application's directory.
    QFile::copy(icon_file, imagePath);
    auto appName = QCoreApplication::applicationName();
    auto appPath = QCoreApplication::applicationFilePath();

    QFile link(destDirectory + QDir::separator() + appName + ".desktop");
    if (link.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&link);
        out.setCodec("UTF-8");
        out << "[Desktop Entry]" << endl
            << "Encoding = UTF-8" << endl
            << "Version  = 1.0" << endl
            << "Type     = Application" << endl
            << "Name     = " << appName << endl
            << "Comment  = " << comment << endl
            << "Icon     = " << imagePath    << endl
            << "Exec     = " << appPath << endl;
        return true;
    }
    return false;
}



} //--- End of qtutils namespace ------//




#endif // QTUTILS_HPP
