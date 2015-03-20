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

/* I would like to investigate the possibility of using a macro
   to call functions with the error param, and ignore-other-than-to-log
   the error if any. Such a macro would take (at least) as arguments:
     - function name
     - error enum name (since they all use the _OK suffix for success)
     - optional variable to store the ret value in
     - the args
*/

/* The following is what I have so far. variadic args alone can be zero
   or more, but having other args besides the variadic means that the
   variadic args must be *one* or more... so a total of four macros for
   two different options. */

#define QTOX_CORE_CALL_RETV_ARGS(func, err_t, retvar, ...) do {        \
    err_t error;                                                       \
    retvar = func(__VA_ARGS__, &error);                                \
    if (error != err_t##_OK)                                           \
        qError() << "Error:" << func << "failed with code" << error;   \
    } while(0)                                                         \

#define QTOX_CORE_CALL_VOID_VOID(func, err_t) do {                     \
    err_t error;                                                       \
    func(&error);                                                      \
    if (error != err_t##_OK)                                           \
        qError() << "Error:" << func << "failed with code" << error;   \
    } while (0)                                                        \

#define QTOX_CORE_CALL_VOID_ARGS(func, err_t, ...) do {                \
    err_t error;                                                       \
    func(__VA_ARGS__, &error);                                         \
    if (error != err_t##_OK)                                           \
        qError() << "Error:" << func << "failed with code" << error;   \
    } while (0)                                                        \

#define QTOX_CORE_CALL_RETV_VOID(func, err_t, retvar) do {             \
    err_t error;                                                       \
    retvar = func(&error);                                             \
    if (error != err_t##_OK)                                           \
        qError() << "Error:" << func << "failed with code" << error;   \
    } while(0)                                                         \

ToxCore::ToxCore(const ToxOptions& options, const QByteArray& data)
{
    
}
