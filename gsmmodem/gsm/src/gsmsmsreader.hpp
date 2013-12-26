/*******************************************************************************
*  file    : gsmsmsreader.hpp
*  created : 11.08.2013
*  author  : 
*******************************************************************************/

#ifndef GSMSMSREADER_HPP
#define GSMSMSREADER_HPP

#include <QObject>
#include <memory>

#include <qatresult.h>
#include <qsmsmessage.hpp>

class GsmSmsReaderPrivate;
class GsmModem;

class QSMSTaggedMessage
{
public:
    QString     identifier;
    QSMSMessage message;
    bool        isUnread;
};

class GsmSmsReader : public QObject
{
    Q_OBJECT
public:
    explicit GsmSmsReader(GsmModem* m, QObject *parent = 0);
    ~GsmSmsReader();
    
    bool ready   () const;
    void setReady(bool ready);

    virtual QString messageStore() const;
    virtual void    setMessageStore(const QString& st);

    const QSMSTaggedMessage* at(int) const;
    int                      count() const;

protected:
    virtual QString messageListCommand() const;
public slots:
    void check();
    void deleteMessage( const QString& id );
private slots:
    void smsReady();
    void nmiStatusReceived( bool ok, const QAtResult& result );
    void newMessageArrived();
    void pduNotification  ( const QString& type, const QByteArray& pdu );
    void cpmsDone         ( bool ok, const QAtResult& result );
    void storeListDone    ( bool ok, const QAtResult& result );
signals:
    void messageCount(int);
    void readyChanged();
private:
    void extractMessages  ( const QString& store, const QAtResult& result );
    void check            (bool force);
    void fetchMessages    ();
    void joinMessages     ();
    bool joinMessages     ( QList<QSMSTaggedMessage*>& messages,
                            QStringList& toBeDeleted );
    bool dispatchDatagram ( QSMSTaggedMessage *m );
private:
    GsmSmsReaderPrivate* d;
};

#endif // GSMSMSREADER_HPP
