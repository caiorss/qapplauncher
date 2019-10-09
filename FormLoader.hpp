#ifndef FORMLOADER_HPP
#define FORMLOADER_HPP

#include <QtWidgets>
#include <QApplication>
#include <QtUiTools/QtUiTools>
#include <QSysInfo>
#include <QtConcurrent/QtConcurrent>

#include <string>

/**
  * Class FormLoader is a helper for loading QtWidgets dynamically
  * from a Form without compilation. */
class FormLoader
{
private:
    QString  formFile;
    QWidget* form;
    QWidget* m_parent;
public:
    FormLoader(QMainWindow* parent, QString path)
    {
        m_parent = parent;

        this->LoadForm(path);
        parent->setCentralWidget(form);
        parent->setWindowTitle(form->windowTitle());

        // Set Width and height
        parent->resize(form->width(), form->height());

        // Center Window in the screen
        parent->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                parent->size(),
                qApp->desktop()->availableGeometry()
                )
            );
    }

    virtual ~FormLoader() = default;

    // Forbid copy, aka Deep Copy
    FormLoader(FormLoader const&) = delete;
    FormLoader& operator=(FormLoader const&) = delete;

    void LoadForm(QString filePath)
    {
        QUiLoader loader;
        formFile = filePath;
        QFile file(filePath);
        // file.open(QFile::ReadOnly);
        form = loader.load(&file, nullptr);
        assert(form != nullptr);
        file.close();
    }

    QWidget* GetForm() { return form;  }

    template<typename T>
    T* find_child(QString widget_name)
    {
        return form->findChild<T*>(widget_name);
    }

    /// Types that models Sender type concept: QPushButton, QCheckBox
    template<typename Sender, typename Callback>
    void on_clicked(QString widget_name, Callback&& event_handler)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        if(pSender == nullptr){
            using namespace std::string_literals;
            throw std::runtime_error("Error: Unable to load widget named: <"s
                                     + widget_name.toStdString()
                                     + "> from the form file "s + formFile.toStdString());
        }
        QObject::connect(pSender, &Sender::clicked, event_handler);
    }

    template<typename Callback>
    void on_button_clicked(QString widget_name, Callback&& event_handler)
    {
        this->on_clicked<QPushButton>(widget_name, event_handler);
    }

    /// Types that models Sender type concept: QListWidget, QTableWidget
    template<typename Sender, typename Callback>
    void on_double_clicked(QString widget_name, Callback&& event_handler)
    {
        Sender* pSender = form->findChild<Sender*>(widget_name);
        assert(pSender != nullptr);
        QObject::connect(pSender, &Sender::doubleClicked, event_handler);
    }

};


#endif // FORMLOADER_HPP
