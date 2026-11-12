
#include "ncbind.hpp"

#define NCB_MODULE_NAME TJS_W("win32dialog.dll")

// Built-in replacement for win32dialog.dll (original:
// https://github.com/wtnbgo/win32dialog). Games only need the modal
// message-box part of the API: `messageBox(window, text, caption, type)`
// returning a Win32 ID* value, plus the MB_*/ID* constants to build the
// `type` argument and compare the result. The dialog-template half of the
// original (Bitmap/Header/Items/open etc.) is not implemented; games using
// it still fail with the corresponding member-not-found error.
//
// Note: this TJS2 port has no `static` member modifier, so the constants
// are attached to the class object via assignment after the class body
// (class-object member assignment works, see packinone/win32dialog probes).
static void InitPlugin_WIN32Dialog() {
    TVPExecuteScript(TJS_W("class WIN32Dialog {") TJS_W(
        "	function messageBox(window, text, caption, type = 0) {\n"
        "		var t = (int)type & 0xf;\n"
        "		var labels, ids;\n"
        "		if (t == WIN32Dialog.MB_YESNO) {\n"
        "			labels = [ \"Yes\", \"No\" ];\n"
        "			ids = [ WIN32Dialog.IDYES, WIN32Dialog.IDNO ];\n"
        "		} else if (t == WIN32Dialog.MB_OKCANCEL) {\n"
        "			labels = [ \"OK\", \"Cancel\" ];\n"
        "			ids = [ WIN32Dialog.IDOK, WIN32Dialog.IDCANCEL ];\n"
        "		} else if (t == WIN32Dialog.MB_ABORTRETRYIGNORE) {\n"
        "			labels = [ \"Abort\", \"Retry\", \"Ignore\" ];\n"
        "			ids = [ WIN32Dialog.IDABORT, WIN32Dialog.IDRETRY, WIN32Dialog.IDIGNORE ];\n"
        "		} else if (t == WIN32Dialog.MB_YESNOCANCEL) {\n"
        "			labels = [ \"Yes\", \"No\", \"Cancel\" ];\n"
        "			ids = [ WIN32Dialog.IDYES, WIN32Dialog.IDNO, WIN32Dialog.IDCANCEL ];\n"
        "		} else if (t == WIN32Dialog.MB_RETRYCANCEL) {\n"
        "			labels = [ \"Retry\", \"Cancel\" ];\n"
        "			ids = [ WIN32Dialog.IDRETRY, WIN32Dialog.IDCANCEL ];\n"
        "		} else {\n"
        "			labels = [ \"OK\" ];\n"
        "			ids = [ WIN32Dialog.IDOK ];\n"
        "		}\n"
        "		var index = System.inform(text, caption, labels);\n"
        "		if (index === null || index === void) index = 0;\n"
        "		return ids[index];\n"
        "	}\n") TJS_W("}") TJS_W(
        "WIN32Dialog.MB_OK = 0;") TJS_W(
        "WIN32Dialog.MB_OKCANCEL = 1;") TJS_W(
        "WIN32Dialog.MB_ABORTRETRYIGNORE = 2;") TJS_W(
        "WIN32Dialog.MB_YESNOCANCEL = 3;") TJS_W(
        "WIN32Dialog.MB_YESNO = 4;") TJS_W(
        "WIN32Dialog.MB_RETRYCANCEL = 5;") TJS_W(
        "WIN32Dialog.MB_CANCELTRYCONTINUE = 6;") TJS_W(
        "WIN32Dialog.MB_ICONERROR = 0x10;") TJS_W(
        "WIN32Dialog.MB_ICONHAND = 0x10;") TJS_W(
        "WIN32Dialog.MB_ICONSTOP = 0x10;") TJS_W(
        "WIN32Dialog.MB_ICONQUESTION = 0x20;") TJS_W(
        "WIN32Dialog.MB_ICONWARNING = 0x30;") TJS_W(
        "WIN32Dialog.MB_ICONEXCLAMATION = 0x30;") TJS_W(
        "WIN32Dialog.MB_ICONINFORMATION = 0x40;") TJS_W(
        "WIN32Dialog.MB_ICONASTERISK = 0x40;") TJS_W(
        "WIN32Dialog.MB_DEFBUTTON1 = 0;") TJS_W(
        "WIN32Dialog.MB_DEFBUTTON2 = 0x100;") TJS_W(
        "WIN32Dialog.MB_DEFBUTTON3 = 0x200;") TJS_W(
        "WIN32Dialog.MB_DEFBUTTON4 = 0x300;") TJS_W(
        "WIN32Dialog.MB_APPLMODAL = 0;") TJS_W(
        "WIN32Dialog.MB_SYSTEMMODAL = 0x1000;") TJS_W(
        "WIN32Dialog.MB_TASKMODAL = 0x2000;") TJS_W(
        "WIN32Dialog.MB_HELP = 0x4000;") TJS_W(
        "WIN32Dialog.MB_SETFOREGROUND = 0x10000;") TJS_W(
        "WIN32Dialog.MB_TOPMOST = 0x40000;") TJS_W(
        "WIN32Dialog.MB_OWNER_CENTER = 0x100000;") TJS_W(
        "WIN32Dialog.IDOK = 1;") TJS_W(
        "WIN32Dialog.IDCANCEL = 2;") TJS_W(
        "WIN32Dialog.IDABORT = 3;") TJS_W(
        "WIN32Dialog.IDRETRY = 4;") TJS_W(
        "WIN32Dialog.IDIGNORE = 5;") TJS_W(
        "WIN32Dialog.IDYES = 6;") TJS_W(
        "WIN32Dialog.IDNO = 7;") TJS_W(
        "WIN32Dialog.IDCLOSE = 8;") TJS_W(
        "WIN32Dialog.IDHELP = 9;") TJS_W(
        "WIN32Dialog.IDTRYAGAIN = 10;") TJS_W(
        "WIN32Dialog.IDCONTINUE = 11;"));
}

NCB_PRE_REGIST_CALLBACK(InitPlugin_WIN32Dialog);
