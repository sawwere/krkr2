//---------------------------------------------------------------------------
// Built-in replacement for PackinOne.dll (append data / archive packing
// protection plugin shipped with a family of commercial releases).
//
// The original DLL extends the global `Storages` class with the members its
// append-data subsystem needs; games only call them after a successful
// `Plugins.link("PackinOne.dll")` (see data/sysscn/append_init.tjs). This
// replacement attaches the same members to the engine's Storages class when
// the module is linked:
//
//   Storages.isExistentDirectory(dir)  -> true/false (dir must end with '/')
//   Storages.dirlist(dir)              -> array of names; directories keep
//                                         their trailing '/' (files too,
//                                         same as dirlist.dll's getDirList)
//
// The hash-name resolution / content descrambling part of the original
// PackinOne is NOT implemented here; archives
// protected that way still fail.
//---------------------------------------------------------------------------
#include "tp_stub.h"
#include "ncbind.hpp"
#include "tjsNative.h"

#include <filesystem>

#define NCB_MODULE_NAME TJS_W("PackinOne.dll")

//---------------------------------------------------------------------------
// Storages.isExistentDirectory
//---------------------------------------------------------------------------
class tIsExistentDirectoryFunction : public tTJSDispatch {
    tjs_error FuncCall(tjs_uint32 flag, const tjs_char *membername,
                       tjs_uint32 *hint, tTJSVariant *result,
                       tjs_int numparams, tTJSVariant **param,
                       iTJSDispatch2 *objthis) {
        if(membername)
            return TJS_E_MEMBERNOTFOUND;

        if(numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        ttstr dir(*param[0]);

        if(dir.GetLastChar() != TJS_W('/'))
            TVPThrowExceptionMessage(
                TJS_W("'/' must be specified at the end of given directory name."));

        // normalize, map to a locally accessible path, then hit the real
        // filesystem; archive-only names map to an empty local path
        dir = TVPNormalizeStorageName(dir);
        TVPGetLocalName(dir);
        const std::string &p = dir.AsNarrowStdString();
        if(result)
            *result = static_cast<tjs_int>(std::filesystem::exists(p) &&
                                           std::filesystem::is_directory(p));

        return TJS_S_OK;
    }
};

//---------------------------------------------------------------------------
// Storages.dirlist
//---------------------------------------------------------------------------
class tDirListFunction : public tTJSDispatch {
    tjs_error FuncCall(tjs_uint32 flag, const tjs_char *membername,
                       tjs_uint32 *hint, tTJSVariant *result,
                       tjs_int numparams, tTJSVariant **param,
                       iTJSDispatch2 *objthis) {
        if(membername)
            return TJS_E_MEMBERNOTFOUND;

        if(numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        ttstr dir(*param[0]);

        if(dir.GetLastChar() != TJS_W('/'))
            TVPThrowExceptionMessage(
                TJS_W("'/' must be specified at the end of given directory name."));

        dir = TVPNormalizeStorageName(dir);

        // create the result Array object
        iTJSDispatch2 *array = TJSCreateArrayObject();
        if(!result)
            return TJS_S_OK;
        try {
            tTJSArrayNI *ni;
            array->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
                                         TJSGetArrayClassID(),
                                         (iTJSNativeInstance **)&ni);
            TVPGetLocalName(dir);
            TVPGetLocalFileListAt(
                dir, [ni](const ttstr &name, tTVPLocalFileInfo *s) {
                    if(s->Mode & (S_IFREG | S_IFDIR)) {
                        ni->Items.emplace_back(name);
                    }
                });
            *result = tTJSVariant(array, array);
            array->Release();
        } catch(...) {
            array->Release();
            throw;
        }

        return TJS_S_OK;
    }
};

//---------------------------------------------------------------------------
// Attach both members to the global Storages native class on link
//---------------------------------------------------------------------------
static void PostRegistCallback() {
    iTJSDispatch2 *global = TVPGetScriptDispatch();
    if(!global)
        return;

    tTJSVariant storages;
    tjs_error hr = global->PropGet(0, TJS_W("Storages"), nullptr, &storages,
                                   global);
    global->Release();
    if(TJS_FAILED(hr))
        return;

    auto *cls = static_cast<tTJSNativeClass *>(storages.AsObjectNoAddRef());
    if(!cls)
        return;

    TJSNativeClassRegisterNCM(cls, TJS_W("isExistentDirectory"),
                              new tIsExistentDirectoryFunction(),
                              cls->getClassName().c_str(), nitMethod,
                              TJS_STATICMEMBER);
    TJSNativeClassRegisterNCM(cls, TJS_W("dirlist"), new tDirListFunction(),
                              cls->getClassName().c_str(), nitMethod,
                              TJS_STATICMEMBER);
}
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
