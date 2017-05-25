#ifndef SPCOMM_H
#define SPCOMM_H

#include <QObject>
#include <QSerialPort>
#include <QVector>

class SPComm : public QObject
{
    Q_OBJECT
public:
    explicit SPComm(QObject *parent = 0);
    ~SPComm();

    static void setPortName(const QString &name);
    static void getAvailablePortName(QVector<QString>&);

    QString portName() const;

    static void setBaudRate(int baudRate);

    int baudRate() const;

    static bool open();

    static void close();

    static bool clear();

    //count必须小于或等于串口返回的数据的长度，不然读取不到数据，返回超时信息
    static int readData(char *buffer, int count, int timeout = 1000);

    static void sleep(unsigned int msec);
    static bool readAllData(char *buffer, int timeout);

    static int writeData(char *data, int size);

    static char getChksum(const char *, char *);

public:

    static QString m_portName;

    static int m_baudRate;

    static QSerialPort *m_serialPort;

signals:

public slots:
};

#endif // SPCOMM_H
