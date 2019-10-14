#ifndef QTUTILS_HPP
#define QTUTILS_HPP

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>

#include <string>
#include <cassert>

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
}


#endif // QTUTILS_HPP
