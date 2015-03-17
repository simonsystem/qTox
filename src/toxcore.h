/* A C++11/Qt5 thin wrapper to the Tox toxcore public API in tox.h.
 * This is a *thin* wrapper, anyone using this is expected to have also
 * read toxcore's header, linked at the bottom of this comment.
 *
 * Having said that, these are some things to know about this wrapper:
 * Since the errors exposed by the core API are largely internal errors,
 * best used for debugging, or external errors that are preventable in clients
 * such that their presence is a bug in the client, it is a design choice that
 * the errors are ignored other than being logged to file, and instead the
 * caller will get an invalid return value (e.g. UINT32_MAX, QString.isNull())
 * In some cases the return value is an enum without an INVALID or similar
 * member of the enum; in such cases the return type is QVariant, which will be
 * QVariant::isNull() on error, or if not, then use QVariant::toInt() and cast
 * to the enum type.
 * Some changes have been made for consistency with Qt conventions. The changes
 * themselves should also be consistent across the API.
 * Note: in some cases, an empty string is valid, so be sure to use isNull()
 * and _not_ isEmpty().
 *
 * Functions are named the same, except in camelCase
 * self_get_nospam becomes getSelfNoSpam, for consistency with Qt conventions
 * Error enums are ignored except for logging purposes
 *   - Return values should instead be tested for success
 * uint8_t arrays for data are converted to/from QByteArrays, and
 * uint8_t arrays for text are converted to/from QStrings
 * Functions that fill an array whose size is provided by another function are wrapped into one,
 *     using the QByteArray/QString objects to handle sizing automatically
 *
 * Otherwise, the documentation from tox.h applies for all functions here
 * https://github.com/irungentoo/toxcore/blob/master/toxcore/tox.h */

#ifndef QTOXCORE_H
#define QTOXCORE_H

#include <tox/tox.h>
#include <QByteArray>
#include <QString>
#include <QList>

class ToxCore : public QObject
{
    Q_OBJECT

public:
    // Fills the argument with the default options
    void optionsDefault(struct Tox_Options* options);

    // creates a default options struct; returns nullptr on failure
    struct Tox_Options* optionsNew();

    // destroys an options struct
    optionsFree(struct Tox_Options* options);

    // the constructor wraps tox_new: data is any save data, null/empty data for new profile
    ToxCore::ToxCore(const struct Tox_Options* options, const QByteArray& data);

    ToxCore::~ToxCore();

    // gets the save data to be written to file
    QByteArray getSaveData() const;

    // returns true on success
    bool bootstrap(const QString& host, uint16_t port, const QByteArray& publicKey);

    // like bootstrap(), but for tcp relays
    bool addTcpRelay(const QString& host, uint16_t port, const QByteArray& publicKey);

    // returns if and how are connected to the DHT
    // this should be avoided in preference to the corresponding signal
    TOX_CONNECTION getConnectionStatus() const;

    // ms before calling iterate()
    uint32_t iterationInterval() const;

    // the main loop, called as iterationInterval dictates
    void iterate();


    /* BEGIN SELF PSEUDO NAMESPACE */

    // get your own Tox friend address
    QByteArray selfGetAddress() const;

    // get your own nospam
    uint32_t getSelfNoSpam() const;

    // set your own nospam
    void setSelfNoSpam(uint32_t nospam);

    // get your own public key
    QByteArray getSelfPublicKey() const;

    // get your own secret key
    QByteArray getSelfSecretKey() const;

    // get your own nickname
    QString getSelfName() const;

    // set your own nickname, returns true on success (limited to TOX_MAX_NAME_LENGTH)
    bool setSelfName(const QString& name);

    // get your own status message
    QString getSelfStatusMessage() const;

    // set your own status message, returns true on success
    bool setSelfStatusMessage(const QString& status);

    // get your user status (online, busy, away)
    TOX_USER_STATUS getSelfStatus() const;

    // set your user status (online, busy, away)
    void setSelfStatus(TOX_USER_STATUS user_status);


    /* BEGIN FRIEND PSEUDO NAMESPACE */

    // Add a friend, message required. INT32_MAX is the maximum number of friends
    // returns the friend number, or UINT32_MAX on failure
    uint32_t friendAdd(const QByteArray& address, const QString& message);

    // (attempts to) connect to the friend without sending a friend request
    // also returns the friendnumber/UINT32_MAX
    uint32_t friendAddNoRequest(const QByteArray& public_key);

    // (silently) removes a friend, returns true on success
    bool friendDelete(uint32_t friend_number);

    // translates a friend's pk to their current friend_number, returns UINT32_MAX on failure
    uint32_t friendByPublicKey(const QByteArray& public_key) const;

    // get the pk for the given friend_number
    QByteArray friendGetPublicKey(uint32_t friend_number) const;

    // check if friend_number is valid
    bool friendExists(uint32_t friend_number) const;

    // get a list of friend_numbers (kinda exists in both namespaces)
    QList<uint32_t> getSelfFriendList() const;

    // get the friend's nickname, returns isNull() string on error
    // note: isEmpty() strings are valid nicks, but not isNull()
    // should probably use the corresponding signal instead of this
    QString friendGetName(uint32_t friend_number) const;

    // get the friend's status message, returns isNull() string on error
    // note: isEmpty() strings are valid nicks, but not isNull()
    // should probably use the corresponding signal instead of this
    QString friendGetStatusMessage(uint32_t friend_number) const;

    // get friend's user status (online, away, busy)
    // use QVariant::toInt and cast to TOX_USER_STATUS enum
    // should probably use the corresponding signal instead of this
    QVariant friendGetStatus(uint32_t friend_number) const;



signals:
    // DHT connection status. Read the tox.h documentation on this callback,
    // handle the signal with care.
    void connectionStatusChanged(TOX_CONNECTION connection_status);

    // friend's name changed
    void friendNameChanged(uint32_t friend_number, QString name);

    // friend's status message changed
    void friendStatusMessageChanged(uint32_t friend_number, QString message);

    // friend's status changed
    void friendStatusChanged(uint32_t friend_number, TOX_USER_STATUS status);

private:
    Tox* tox;

};

#endif // QTOXCORE_H
