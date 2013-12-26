/*******************************************************************************
*  file    : gsmsmsmessage.hpp
*  created : 11.08.2013
*  author  : 
*******************************************************************************/

#ifndef GSMSMSMESSAGE_HPP
#define GSMSMSMESSAGE_HPP

#include <qstring.h>
#include <qdatetime.h>
#include <qdatastream.h>
#include <qlist.h>
#include <quuid.h>


class QSMSMessagePartPrivate;
class QSMSMessagePrivate;
class QTextCodec;


class QSMSMessagePart
{
public:
    QSMSMessagePart();
    explicit QSMSMessagePart( const QString& text );
    QSMSMessagePart( const QString& mimeType, const QByteArray& data );
    QSMSMessagePart( const QString& mimeType, const QByteArray& data, uint position );
    QSMSMessagePart( const QSMSMessagePart& part );
    ~QSMSMessagePart();

    QSMSMessagePart& operator=( const QSMSMessagePart& part );

    bool isText() const;
    QString text() const;
    QString mimeType() const;
    const QByteArray& data() const;
    uint position() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSMSMessagePartPrivate *d;
};


enum QSMSDataCodingScheme {
    QSMS_Compressed      = 0x0020,
    QSMS_MessageClass    = 0x0010,
    QSMS_DefaultAlphabet = 0x0000,
    QSMS_8BitAlphabet    = 0x0004,
    QSMS_UCS2Alphabet    = 0x0008,
    QSMS_ReservedAlphabet= 0x000C
};

class QSMSMessage
{
    friend class QSMSSubmitMessage;
    friend class QSMSDeliverMessage;
public:
    QSMSMessage();
    QSMSMessage(const QSMSMessage &);
    ~QSMSMessage();

    QSMSMessage& operator=(const QSMSMessage &);

    void setText(const QString &);
    QString text() const;

    void setTextCodec(QTextCodec *codec);
    QTextCodec *textCodec() const;

    void setForceGsm(bool force);
    bool forceGsm() const;

    void setBestScheme(QSMSDataCodingScheme);
    QSMSDataCodingScheme bestScheme() const;

    void setRecipient(const QString &);
    QString recipient() const;

    void setSender(const QString &);
    QString sender() const;

    void setServiceCenter(const QString &);
    QString serviceCenter() const;

    void setReplyRequest(bool on );
    bool replyRequest() const;

    void setStatusReportRequested(bool on );
    bool statusReportRequested() const;

    void setValidityPeriod(uint minutes);
    uint validityPeriod() const;

    void setGsmValidityPeriod(uint value);
    uint gsmValidityPeriod() const;

    void setTimestamp(const QDateTime &);
    QDateTime timestamp() const;

    void setHeaders(const QByteArray& value);
    const QByteArray& headers() const;

    void clearParts();
    void addPart( const QSMSMessagePart& part );
    void addParts( const QList<QSMSMessagePart>& parts );
    QList<QSMSMessagePart> parts() const;

    enum MessageType {
        Normal, CellBroadCast, StatusReport
    };

    MessageType messageType() const;

    void computeSize( uint& numMessages, uint& spaceLeftInLast ) const;

    int destinationPort() const;
    void setDestinationPort(int value);

    int sourcePort() const;
    void setSourcePort(int value);

    QByteArray applicationData() const;
    void setApplicationData(const QByteArray& value);

    void setDataCodingScheme(int);
    int dataCodingScheme() const;

    void setMessageClass(int);
    int messageClass() const;

    void setProtocol(int);
    int protocol() const;

    bool shouldSplit() const;

    QList<QSMSMessage> split() const;

    QByteArray toPdu() const;
    static QSMSMessage fromPdu( const QByteArray& pdu );
    static int pduAddressLength( const QByteArray& pdu );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

protected:
    void setMessageType(MessageType);

private:
    mutable QSMSMessagePrivate *d;

    QSMSMessagePrivate *dwrite();

    int  findPart( const QString& mimeType ) const;
    void removeParts( const QString& mimeType );
    void setFragmentHeader( uint refNum, uint part, uint numParts,
                            QSMSDataCodingScheme scheme );
    void unpackHeaderParts();
};


#endif // GSMSMSMESSAGE_HPP
