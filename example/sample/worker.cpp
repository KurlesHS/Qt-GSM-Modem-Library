#include "worker.h"
#include "gsmmodem.hpp"
#include "gsmsmsreader.hpp"
#include "gsmsmssender.hpp"
#include "qtelephonynamespace.h"
#include <QDebug>
#include <QCoreApplication>
#include <QUuid>
Worker::Worker(const QString &serialPort, QObject *parent) :
    QObject(parent),
    m_modem(new GsmModem("*100#", this)),
    m_reader(new GsmSmsReader(m_modem, this)),
    m_sender(new GsmSmsSender(m_modem, this))
{
    m_modem->init(serialPort, 9600);

    connect(m_reader, SIGNAL(messageCount(int)), this, SLOT(onMessageCount(int)));
    connect(m_sender, SIGNAL(startSendingSms(QString)), this, SLOT(onStartSendingSms(QString)));
    connect(m_sender, SIGNAL(finished(QString,int)), this, SLOT(onFinished(QString,int)));
    connect(m_modem, SIGNAL(initialized(bool)), this, SLOT(onInitialized(bool)));
    connect(m_modem, SIGNAL(balanceCash(double)), this, SLOT(onBalanceCash(double)));
    connect(m_modem, SIGNAL(operatorName(QString)), this, SLOT(onOperatorName(QString)));
    connect(m_modem, SIGNAL(signalLevels(int,int)), this, SLOT(onSignalLevel(int,int)));
}

void Worker::onInitialized(bool initialized)
{
    if (initialized) {
        qDebug() << "Modem is initialized";
        m_reader->check();

        QSMSMessage sms;
        sms.setText("Hello");
        sms.setRecipient("+7xxxxxxxxxx");
        sms.setStatusReportRequested(true);
        m_sender->send(QUuid::createUuid().toString(), sms);

    } else {
        qDebug() << "Failed to initialize modem";
        qApp->exit(-1);
    }
}

void Worker::onBalanceCash(double balance)
{
    qDebug() << "Balance:" << balance;
}

void Worker::onOperatorName(QString name)
{
    qDebug() << "operator name:" << name;
}

void Worker::onSignalLevel(int signalLevel, int errorLevel)
{
    qDebug() << QString("Signal level: %0, error level: %1").arg(signalLevel).arg(errorLevel);
}

void Worker::onMessageCount(int count)
{
    Q_UNUSED(count)
    readSmsMessages(false);
}

void Worker::onStartSendingSms(const QString &id)
{
    qDebug() << QString("Start sending sms whit id %0").arg(id);
}

void Worker::onFinished(const QString &id, const int &code)
{
    if (code == QTelephony::OK) {
        qDebug() << QString("Sms with id %0 is sended")
                    .arg(id);
    } else if (code == QTelephony::Delivered) {
        qDebug() << QString("Sms with id %0 is delivered")
                    .arg(id);
    } else {
        qDebug() << QString("something terrible happens while sending sms with id %0")
                    .arg(id);
    }
}

void Worker::readSmsMessages(const bool deleteSmsAfterRead)
{
    QStringList smsIdList;
    for (int i = 0; i < m_reader->count(); ++i) {
        const QSMSTaggedMessage *message = m_reader->at(i);
        if (message) {
            qDebug() << QString("Message '%0' from '%1' received")
                        .arg(message->message.text())
                        .arg(message->message.sender());
            if (deleteSmsAfterRead)
                smsIdList << message->identifier;
        }
    }
    for (const QString &id: smsIdList) {
        m_reader->deleteMessage(id);
    }
}
