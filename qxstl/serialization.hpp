#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

//--- STL Headers --//
#include <memory> // std::unique_ptr, std::shared_ptr

//--- QT Headers ---//
#include <QtCore>

namespace qxstl::serialization
{

template<typename T>
QVariant value_writer(T& value)
{
    return value;
}

template<typename T>
QVariant value_writer(QList<T>& value)
{
    QByteArray arr;
    QDataStream ss{&arr, QIODevice::WriteOnly};
    ss << value;
    return arr;
}

template<typename T>
inline void value_reader(T& ref, QVariant value);

template<>
inline void value_reader(int& ref, QVariant value)
{
    ref = value.toInt();
}

template<>
inline void value_reader(double& ref, QVariant value)
{
    ref = value.toDouble();
}

template<>
inline void value_reader(QString& ref, QVariant value)
{
    ref = value.toString();
}

template<>
inline void value_reader(QVariant& ref, QVariant value)
{
    ref = value;
}

template<>
inline void value_reader(QStringList& ref, QVariant value)
{
    ref = value.toStringList();
}


// Note 1: The QDatStream object is not owned by this class
// Note 2: The class inheriting this one must call flush in the destructor
//         for ensuring that data was written to the stream.
struct StreamWriter
{
private:
    // std::unique_ptr<QFile>       pfile;
    QDataStream*             pss;
    QMap<QString, QVariant>  dataset;

public:
    StreamWriter() { }
    StreamWriter(QDataStream* pss):
        pss{pss}
    {
        pss->setVersion(QDataStream::Qt_5_0);
    }

    virtual ~StreamWriter()
    {
    }

    void set_stream(QDataStream* pss)
    {
        this->pss = pss;
    }

    template<typename Visitable>
    void operator()(Visitable&& visitable)
    {
        visitable.accept(*this);
        (*pss) << dataset;
        dataset.clear();
    }

    template<typename T>
    void visit(QString name, T& value)
    {
        dataset[name] = value_writer<T>(value);
    }

};


struct FileWriter: public StreamWriter
{
private:
    std::unique_ptr<QFile>       file;
    std::unique_ptr<QDataStream> dts;

public:
    FileWriter(QString file_name)
        : StreamWriter{}
    {
        file = std::make_unique<QFile>(file_name);
        if(!file->open(QIODevice::WriteOnly))
        {
            throw std::runtime_error(" [ERROR] Cannot open file.");
        }
        dts = std::make_unique<QDataStream>(file.get());
        //this->StreamWriter::set_stream(dts.get());
        this->set_stream(dts.get());
    }
};

struct StreamReader
{
private:
    // std::unique_ptr<QFile>       pfile;
    // std::unique_ptr<QDataStream> pss;
    QDataStream*             pss;
    QMap<QString, QVariant>  dataset;

public:
    //StreamReader(QString file_name)
    StreamReader() {}

    StreamReader(QDataStream* pss)
    {
        pss->setVersion(QDataStream::Qt_5_0);
        (*pss) >> dataset;
    }

    void set_stream(QDataStream* pss)
    {
        this->pss = pss;
        pss->setVersion(QDataStream::Qt_5_0);
    }

    template<typename Visitable>
    void operator()(Visitable& visitable)
    {
        (*pss) >> dataset;
        visitable.accept(*this);
        dataset.clear();
    }

    template<typename T>
    void visit(QString name, T& ref)
    {
        QVariant x = dataset[name];
        value_reader(ref, x);
    }

    template<typename T>
    void visit(QString name, QList<T>& value)
    {
        QByteArray arr = dataset[name].toByteArray();
        QDataStream ss{&arr, QIODevice::ReadOnly};
        ss >> value;
    }
};

struct FileReader: public StreamReader
{
    std::unique_ptr<QFile> pfile;
    std::unique_ptr<QDataStream> pss;

    FileReader(QString file_name)
    {
        pfile = std::make_unique<QFile>(file_name);
        if(!pfile->open(QIODevice::ReadOnly))
        {
            throw std::runtime_error(" [ERROR] Cannot open the file.");
        }
        pss = std::make_unique<QDataStream>(pfile.get());
        this->StreamReader::set_stream(pss.get());
    }
};

}

#endif // SERIALIZATION_HPP
