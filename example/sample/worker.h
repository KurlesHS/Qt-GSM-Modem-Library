#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class GsmModem;
class GsmSmsReader;
class GsmSmsSender;

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(const QString &serialPort, QObject *parent = 0);

signals:

public slots:
    void onInitialized(bool initialized);
    void onBalanceCash(double balance);
    void onOperatorName(QString name);
    void onSignalLevel(int signalLevel, int errorLevel);

    void onMessageCount(int count);

    void onStartSendingSms(const QString &id);
    void onFinished(const QString &id, const int &code);

private:
    void readSmsMessages(const bool deleteSmsAfterRead = false);


public:
    GsmModem *m_modem;
    GsmSmsReader *m_reader;
    GsmSmsSender *m_sender;
};

#endif // WORKER_H
