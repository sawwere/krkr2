//---------------------------------------------------------------------------
// Built-in replacement for GlitchEffect.dll (screen-glitch visual plugin
// shipped with one such release).
//
// The original extends the Layer class with a glitch-distorted copy:
//   layer.glitchCopy(x, y, srcLayer, sx, sy, w, h, param)
// where `param` (noise/size/sft_x/sft_y/coef keys) drives the distortion.
// The game calls it unconditionally on its title-screen animation paths
// (uimain.tjs doGlitch/updateGlitch), so the member must exist even though
// the aesthetic is not reproduced: this replacement forwards the copy
// geometry to Layer.copyRect and ignores the distortion parameter (plain
// copy fallback). All other glitch* identifiers the game touches
// (kag.glitchSysTrans, glitchParam, ...) are game-side TJS objects.
//---------------------------------------------------------------------------
#include "tp_stub.h"
#include "ncbind.hpp"
#include "tjsNative.h"

#define NCB_MODULE_NAME TJS_W("GlitchEffect.dll")

//---------------------------------------------------------------------------
// Layer.glitchCopy — plain-copy fallback via the same layer's copyRect
//---------------------------------------------------------------------------
class tGlitchCopyFunction : public tTJSDispatch {
    tjs_error FuncCall(tjs_uint32 flag, const tjs_char *membername,
                       tjs_uint32 *hint, tTJSVariant *result,
                       tjs_int numparams, tTJSVariant **param,
                       iTJSDispatch2 *objthis) {
        if(membername)
            return TJS_E_MEMBERNOTFOUND;

        if(!objthis)
            return TJS_E_NATIVECLASSCRASH;

        if(numparams < 7)
            return TJS_E_BADPARAMCOUNT;

        if(result)
            result->Clear();

        // copyRect(x, y, src, sx, sy, w, h); param[7] (distortion) ignored
        tTJSVariant unused;
        return objthis->FuncCall(0, TJS_W("copyRect"), nullptr, &unused, 7,
                                 param, objthis);
    }
};

//---------------------------------------------------------------------------
// Attach glitchCopy to the global Layer native class on link
//---------------------------------------------------------------------------
static void PostRegistCallback() {
    iTJSDispatch2 *global = TVPGetScriptDispatch();
    if(!global)
        return;

    tTJSVariant layerval;
    tjs_error hr =
        global->PropGet(0, TJS_W("Layer"), nullptr, &layerval, global);
    global->Release();
    if(TJS_FAILED(hr))
        return;

    auto *cls = static_cast<tTJSNativeClass *>(layerval.AsObjectNoAddRef());
    if(!cls)
        return;

    TJSNativeClassRegisterNCM(cls, TJS_W("glitchCopy"),
                              new tGlitchCopyFunction(),
                              cls->getClassName().c_str(), nitMethod, 0);
}
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
