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

#include "toxcore.h"
#include <new>
#include <QDebug>

/* First, some utility functions for reducing code duplication, and macros
   to simplify their usage. Thank fscking god for variadic templates.
*/

/* Use a nifty template to heavily reduce code duplication.
   It wraps any core function with an error checker/logger
   (itself wrapped in a macro to get the function name for the error log).
   Sadly it can't be used for functions with undefined retval on error, since the
   caller doesn't see any errors.
   Many thanks to nurupo and iphy for much help with the templating
*/

template <typename R, typename E, typename... ARGS>
static R qtox_call_core(const char* name, R (*func)(ARGS..., E*), E ok_val, ARGS... args)
{
    E error;
    R ret = func(args, &error);
    if (error != ok_val)
        qError() << "Error:" << name << "failed with code" << error;
    return ret;
}
#define QTOX_CALL_CORE(FUNC, OK, ...) qtox_call_core(#FUNC, FUNC, OK, __VA_ARGS__)

static typedef enum {CLAMP_LTE, CLAMP_EQ, CLAMP_GTE} CLAMP_TYPE;
static bool clamp(size_t sz, size_t req, CLAMP_TYPE type, const char* func, int line)
{
    if (type == CLAMP_LTE)
    {
        if (sz <= req)
        {
            return true;
        }
    }
    else if (type == CLAMP_EQ)
    {
        if (sz == req)
        {
            return true;
        }
    }
    else if (type == CLAMP_GTE)
    {
        if (sz >= req)
        {
            return true;
        }            
    }
    else
    {
        qError() << "Error: unknown clamp type in" << func << "at line" << line;
        return false;
    }
    qError() << "Error:" << func << "failed a clamp at line" << line << "sizes:" << sz << req;
    return false;
}
#define CLAMP(sz, req, type) clamp(sz, req, type, __func__, __LINE__)
// C++11 defines __func__ (as a local variable) and __LINE__ (a macro)
// To make best use of this, only allow one call per line

// I'm not copying and pasting this, much less typing it each time
#define QBA_CAST(data) (reinterpret_cast<const char*>((data)))
#define UI8_CAST(data) (reinterpret_cast<uint8_t*>   ((data)))

// ditto
#define CORE_CAST(ptr) (static_cast<ToxCore*>((data)))

////////////////////////////////////////////////////////////////////////////////
// Now the implementation

ToxOptions::ToxOptions()
{
    tox_options = tox_options_new(); // nullptr on malloc failure
    if (!tox_options) // emulate new operator
    {
        qError() << "Amazing. You actually broke malloc.";
        throw std::bad_alloc;
    }
}

ToxOptions::~ToxOptions()
{
    tox_options_free(tox_options);
}

// clears this and sets it to default opts
void ToxOptions::optionsDefault()
{
    tox_options_default(tox_options);
}

bool ToxOptions::isNull() const
{
    return tox_options == nullptr;
}

QByteArray ToxCore::getSaveData() const
{
    QByteArray ret;
    ret.resize(tox_get_savedata_size(tox));
    tox_get_savedata(tox, UI8_CAST(ret.data()));
    return ret;
}

bool ToxCore::bootstrap(const QString& host, uint16_t port, const QByteArray& publicKey)
{
    if (   !CLAMP(host.size(), 255, CLAMP_LTE)
        || !CLAMP(publicKey.size(), TOX_PUBLIC_KEY_SIZE, CLAMP_EQ))
            return false;
    CString hst(host);
    CString pk(publicKey);
    return QTOX_CALL_CORE(tox_bootstrap, TOX_ERR_BOOTSTRAP_OK, tox, hst.data(), port, pk.data());
}

bool ToxCore::addTcpRelay(const QString& host, uint16_t port, const QByteArray& publicKey)
{
    if (   !CLAMP(host.size(), 255, CLAMP_LTE)
        || !CLAMP(publicKey.size(), TOX_PUBLIC_KEY_SIZE, CLAMP_EQ))
            return false;
    CString hst(host);
    CString pk(publicKey);
    return QTOX_CALL_CORE(tox_add_tcp_relay, TOX_ERR_BOOTSTRAP_OK, tox, hst.data(), port, pk.data());
}

TOX_CONNECTION ToxCore::getSelfConnectionStatus() const
{
    return tox_self_get_connection_status(tox);
}

static void selfConnectionStatusChanged(Tox*, TOX_CONNECTION status, void* data)
{
    emit CORE_CAST(data)->selfConnectionStatusChanged(status);
}

uint32_t ToxCore::iterationInterval() const;
{
    return tox_iteration_interval(tox);
}

void ToxCore::iterate()
{
    tox_iterate(tox);
}

QByteArray ToxCore::getSelfAddress() const
{
    QByteArray ret;
    ret.resize(TOX_ADDRESS_SIZE);
    tox_self_get_address(tox, UI8_CAST(ret.data()));
    return ret;
}

uint32_t ToxCore::getSelfNoSpam() const
{
    return tox_self_get_nospam(tox);
}

void ToxCore::setSelfNoSpam(uint32_t nospam) const
{
    tox_self_set_nospam(tox, nospam);
}

QByteArray ToxCore::getSelfPublicKey() const
{
    QByteArray ret;
    ret.resize(TOX_PUBLIC_KEY_SIZE);
    tox_self_get_public_key(tox, UI8_CAST(ret.data()));
    return ret;
}

QByteArray getSelfSecretKey() const
{
    QByteArray ret;
    ret.resize(TOX_SECRET_KEY_SIZE];
    tox_self_get_secret_key(tox, UI8_CAST(ret.data()));
    return ret;
}

QByteArray getSelfName() const
{
    QByteArray ret;
    ret.resize(tox_self_get_name_size(tox));
    tox_self_get_name(tox, UI8_CAST(ret.data()));
    return ret;
}



ToxCore::ToxCore(const ToxOptions& options, const QByteArray& data)
{
    //TODO
}

ToxCore::~ToxCore()
{
    tox_kill(tox);
}
