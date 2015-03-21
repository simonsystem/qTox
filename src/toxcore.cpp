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

/* Use a nifty template to heavily reduce code duplication.
   It wraps any core function with an error checker/logger
   (itself wrapped in a macro to get the function name for the error log).
   Sadly it can't be used for functions with undefined retval on error, since the
   caller doesn't see any errors.
   Many thanks to nurupo and iphy for much help with the templating
*/

template <typename R, typename E, typename... ARGS>
R qtox_call_core(const char* name, R (*func)(ARGS..., E*), E ok_val, ARGS... args)
{
    E error;
    R ret = func(args, &error);
    if (error != ok_val)
        qError() << "Error:" << name << "failed with code" << error;
    return ret;
}

#define QTOX_CALL_CORE(FUNC, OK, ...) qtox_call_core(#FUNC, FUNC, OK, __VA_ARGS__)

ToxCore::ToxCore(const ToxOptions& options, const QByteArray& data)
{
    //TODO
}

QByteArray ToxCore::getSaveData() const
{
    size_t size = tox_get_savedata_size(tox);
    uint8_t* array = new uint8_t[size];
    //TODO
}

bool ToxCore::bootstrap(const QString& host, uint16_t port, const QByteArray& publicKey)
{
    CString s(host);
    CString b(publicKey);
    return QTOX_CALL_CORE(tox_bootstrap, TOX_ERR_BOOTSTRAP_OK, tox, s.data(), port, b.data());
}
