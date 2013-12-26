/*******************************************************************************
*  file    : gsmsmssender.hpp
*  created : 13.08.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#ifndef GSMSMSSENDER_HPP
#define GSMSMSSENDER_HPP

#include <QObject>
#include <qsmsmessage.hpp>
#include <qatresult.h>

class GsmModem;
class GsmSmsSenderPrivate;
class QAtResult;

class GsmSmsSender : public QObject
{
    Q_OBJECT
public:
    explicit GsmSmsSender( GsmModem *service ,QObject *parent = 0);
    ~GsmSmsSender();
    int smsInProcess() const;

public slots:
    void send( const QString& id, const QSMSMessage& msg );

private slots:
    void smsReady();
    void smsReadyTimeout();
    void sendNext();
    void transmitDone( bool ok, const QAtResult& result );
    void onNotification(QString type, QString value);
    void onPduNotification(QString name ,QByteArray pdu);

signals:
    void finished(const QString& , int);
    void startSendingSms(const QString &id);

private:
    GsmSmsSenderPrivate *d;
};

#endif // GSMSMSSENDER_HPP
