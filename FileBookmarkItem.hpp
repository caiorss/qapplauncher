#ifndef FILEBOOKMARKITEM_HPP
#define FILEBOOKMARKITEM_HPP

#include <QtCore>

struct FileBookmarkItem
{
    QString uri_path;
    QString brief;
    QString description;

    FileBookmarkItem(){}
    FileBookmarkItem(QString uri_path, QString brief, QString description):
        uri_path(uri_path), brief(brief), description(description)
    {}
};


#endif // FILEBOOKMARKITEM_HPP
