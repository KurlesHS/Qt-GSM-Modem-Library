/*******************************************************************************
*  file    : gsmsmssender.cpp
*  created : 13.08.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#include "gsmsmssender.hpp"

#include <gsmmodem.hpp>
#include <qatresult.h>
#include <qatchat.h>
#include <qtelephonynamespace.h>
#include <QTimer>
#include <QDebug>

/*!
    \class QModemSMSSender
    \inpublicgroup QtCellModule

    \brief The QModemSMSSender class provides SMS sending facilities for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CMGS} and \c{AT+CMMS} commands from
    3GPP TS 27.005.

    QModemSMSSender implements the QSMSSender telephony interface.  Client
    applications should use QSMSSender instead of this class to
    access the modem's SMS sending facilities.

    \sa QSMSSender
*/

class QModemSMSSenderPending
{
public:
    QString id;
    QList<QSMSMessage> msgs;
    int posn;
    QModemSMSSenderPending *next;
    QTelephony::Result result;
    int retries;
};

class GsmSmsSenderPrivate
{
public:
    GsmSmsSenderPrivate( GsmModem *service )
    {
        this->service = service;
        this->first = 0;
        this->last = 0;
        this->sending = false;
    }

    GsmModem*               service;
    QModemSMSSenderPending* first;
    QModemSMSSenderPending* last;
    QTimer*                 needTimeout;
    QHash<int, QString>     waitingDeliveryReportSmsHash;
    bool                    sending;
};

/*!
    Create a new AT-based SMS sending object for for \a service.
*/
GsmSmsSender::GsmSmsSender(  GsmModem *service ,QObject *parent )
    : QObject(parent)
{
    d = new GsmSmsSenderPrivate( service );
    //service->connectToPost( "smsready", this, SLOT(smsReady()) );

    d->needTimeout = new QTimer( this );
    d->needTimeout->setSingleShot( true );
    connect( d->needTimeout, SIGNAL(timeout()), this, SLOT(smsReadyTimeout()) );
    connect( service->primaryAtChat(), SIGNAL(notification(QString,QString)),
             this, SLOT(onNotification(QString,QString)) );
    connect( service->primaryAtChat(), SIGNAL(pduNotification(QString,QByteArray)),
             this,SLOT(onPduNotification(QString,QByteArray)) );
}

/*!
    Destroy this SMS sending object.
*/
GsmSmsSender::~GsmSmsSender()
{
    delete d;
}

int GsmSmsSender::smsInProcess() const
{
    int initialCount = 0;
    QModemSMSSenderPending *first = d->first;
    if (first) {
        ++initialCount;
        first = first->next;
    }
    return initialCount;
}

/*!
    \reimp
*/
void GsmSmsSender::send( const QString& id, const QSMSMessage& msg )
{
    // Create the message block and add it to the pending list.
    QModemSMSSenderPending *pending = new QModemSMSSenderPending();
    pending->id      = id;
    pending->posn    = 0;
    pending->result  = QTelephony::OK;
    pending->retries = 5;
    pending->msgs    = msg.split();    // Split into GSM-sized message fragments.
    pending->next    = 0;
    if ( d->last )
        d->last->next = pending;
    else
        d->first = pending;
    d->last = pending;

    // If this was the first message, then start the transmission process.
    if ( d->first == pending )
        {
            d->needTimeout->start( 15 * 1000 );
            sendNext();
        }
}

void GsmSmsSender::smsReady()
{
    // Stop the "need SMS" timeout if it was active.
    d->needTimeout->stop();

    // Bail out if no messages to send or we are already sending.
    // QModemSMSReader may have caused the "smsReady()" signal to
    // be emitted, so this is a false positive.
    if ( !d->first || d->sending )
        return;

    // Transmit the next message in the queue.
    sendNext();
}

void GsmSmsSender::smsReadyTimeout()
{
    // Fail all of the pending requests as the SMS system is not available.
    QModemSMSSenderPending *current = d->first;
    QModemSMSSenderPending *next;
    while ( current != 0 )
        {
            next = current->next;
            emit finished( current->id, QTelephony::SMSOperationNotAllowed );
            delete current;
            current = next;
        }
    d->first = 0;
    d->last = 0;
}

void GsmSmsSender::sendNext()
{
    // We are currently sending.
    d->sending = true;
    d->needTimeout->start( 15 * 1000 );

    // Emit a finished signal if the current message is done.
    QModemSMSSenderPending *current = d->first;
    if ( !d->first )
        {
            d->sending = false;
            return;
        }

    if ( current->posn >= current->msgs.size() )
        {
            emit finished( current->id, current->result );
            d->first = current->next;
            if ( !d->first )
                d->last = 0;
            delete current;
            if ( !d->first )
                {
                    d->sending = false;
                    return;
                }
            current = d->first;
    }
    if ( current->posn == 0 ) {
        emit startSendingSms(current->id);
    }
    // If this is the first message in a multi-part, send AT+CMMS=1.
    if ( current->posn == 0 && current->msgs.size() > 1 ) {
        d->service->chat( "AT+CMMS=1" );
    }

    // Transmit the next message in the queue.
    QSMSMessage m = current->msgs[ (current->posn)++ ];

    // Convert the message into a hexadecimal PDU string.
    QByteArray pdu = m.toPdu();

    // Get the length of the data portion of the PDU,
    // excluding the service centre address.
    uint pdulen = pdu.length() - QSMSMessage::pduAddressLength( pdu );

    // Build and deliver the send command.  Note: "\032" == CTRL-Z.
    QString cmd = "AT+CMGS=" + QString::number( pdulen );
    d->service->primaryAtChat()->chatPDU
        ( cmd, pdu, this, SLOT(transmitDone(bool,QAtResult)) );
}

void GsmSmsSender::transmitDone( bool ok, const QAtResult& result )
{
    // If the send was successful, then move on to the next message to be sent.
    if ( ok )
        {
            sendNext();
            return;
        }

    // If this is the first or only message in a multi-part,
    // then retry after a second.  The system may not be ready.
    QModemSMSSenderPending *current = d->first;
    if ( current->posn == 1 )
        {
            if ( --(current->retries) > 0 )
                {
                    --(current->posn);
                    QTimer::singleShot( 1000, this, SLOT(sendNext()) );
                    return;
                }
        }

    // Record the error and then move on to the next message.
    current->result = (QTelephony::Result)( result.resultCode() );
    sendNext();
}

void GsmSmsSender::onNotification( QString type, QString value )
{
    if ( type == QString("+CMGS:") ) {
        QModemSMSSenderPending *current = d->first;
        if (current) {
            d->waitingDeliveryReportSmsHash[value.toInt()] = current->id;
        }
    }
}

void GsmSmsSender::onPduNotification(QString name, QByteArray pdu)
{
    if ( name.size() >= 6 && name.left( 5 ) == QString( "+CDS:" ) ) {
        int index = 1;
        // skip sms center number
        index += pdu[0] & 0xff;
        // skip flags
        ++index;

        if (pdu.size() < index + 1)
            return;

        int internalModemSmsId = pdu[index] & 0xff;
        QString smsId = d->waitingDeliveryReportSmsHash.value( internalModemSmsId, QString() );
        if ( !smsId.isEmpty() ) {
            d->waitingDeliveryReportSmsHash.remove( internalModemSmsId );
            emit finished( smsId, QTelephony::Delivered );
        }
    }
}
