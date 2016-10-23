#include <nan.h>

using namespace v8;

static WCHAR *windowClassName = L"DRAWERWINDOW";

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL initWindowClass(void){
	WNDCLASSW wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.lpszClassName = windowClassName;
	if( !RegisterClassW(&wndClass) ){
		return false;
	} else {
		return true;
	}
}

static HWND create_window(){
	return CreateWindowW(windowClassName, L"Dummy Window", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), NULL);
}

static BOOL dispose_window(HWND hwnd){
	return DestroyWindow(hwnd);
}

static void parse_devnames(DEVNAMES *devnames, WCHAR **driver, WCHAR **device, WCHAR **output)
{
	*driver = (WCHAR *)(((WCHAR *)devnames) + devnames->wDriverOffset);
	*device = (WCHAR *)(((WCHAR *)devnames) + devnames->wDeviceOffset);
	*output = (WCHAR *)(((WCHAR *)devnames) + devnames->wOutputOffset);
}

void createWindow(const Nan::FunctionCallbackInfo<Value>& args){
	// createWidnow()
	HWND hwnd = create_window();
	if( hwnd == NULL ){
		printf("%d\n", GetLastError());
		Nan::ThrowTypeError("create_window failed");
		return;
	}
	args.GetReturnValue().Set(Nan::New((int)hwnd));
}

void disposeWindow(const Nan::FunctionCallbackInfo<Value>& args){
	// disposeWindow(hwnd)
	if( args.Length() < 1 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}
	if( !args[0]->IsInt32() ){
		Nan::ThrowTypeError("wrong argument");
		return;
	}
	HWND hwnd = (HWND)args[0]->Int32Value();
	BOOL ok = dispose_window(hwnd);
	args.GetReturnValue().Set(Nan::New(ok));
}

void getDc(const Nan::FunctionCallbackInfo<Value>& args){
	// getDc(hwnd)
	if( args.Length() < 1 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}
	if( !args[0]->IsInt32() ){
		Nan::ThrowTypeError("wrong argument");
		return;
	}
	HWND hwnd = (HWND)args[0]->Int32Value();
	HDC hdc = GetDC(hwnd);
	args.GetReturnValue().Set(Nan::New((int)hdc));
}

void releaseDc(const Nan::FunctionCallbackInfo<Value>& args){
	// releaseDc(hwnd, hdc)
	if( args.Length() < 2 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}
	if( !args[0]->IsInt32() || !args[1]->IsInt32() ){
		Nan::ThrowTypeError("wrong arguments");
		return;
	}
	HWND hwnd = (HWND)args[0]->Int32Value();
	HDC hdc = (HDC)args[1]->Int32Value();
	BOOL ok = ReleaseDC(hwnd, hdc);
	args.GetReturnValue().Set(Nan::New(ok));
}

void measureText(const Nan::FunctionCallbackInfo<Value>& args){
	// measureText(hdc, string) => { cx:..., cy:... }
	if( args.Length() < 2 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}
	if( !args[0]->IsInt32() ){
		Nan::ThrowTypeError("wrong argument");
		return;
	}
	if( !args[1]->IsString() ){
		Nan::ThrowTypeError("wrong argument");
		return;
	}
	HDC hdc = (HDC)args[0]->Int32Value();
	String::Value textValue(args[1]);
	SIZE mes;
	BOOL ok = GetTextExtentPoint32W(hdc, (LPCWSTR)*textValue, textValue.length(), &mes);
	if( !ok ){
		Nan::ThrowTypeError("GetTextExtentPoint32W failed");
		return;
	}
	Local<Object> obj = Nan::New<Object>();
	obj->Set(Nan::New("cx").ToLocalChecked(), Nan::New(mes.cx));
	obj->Set(Nan::New("cy").ToLocalChecked(), Nan::New(mes.cy));
	args.GetReturnValue().Set(obj);
}

void createFont(const Nan::FunctionCallbackInfo<Value>& args){
	// createFont(fontname, size, weight?, italic?) ==> HANDLE
	if( args.Length() < 2 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}
	if( !args[0]->IsString() ){
		Nan::ThrowTypeError("invalid font name");
		return;
	}
	if( !args[1]->IsInt32() ){
		Nan::ThrowTypeError("invalid font size");
		return;
	}
	if( args.Length() >= 3 && !args[2]->IsInt32() ){
		Nan::ThrowTypeError("invalid font weight");
		return;
	}
	if( args.Length() >= 4 && !args[3]->IsInt32() ){
		Nan::ThrowTypeError("invalid font italic");
		return;
	}
	String::Value fontName(args[0]);
	long size = args[1]->Int32Value();
	long weight = args.Length() >= 3 ? args[2]->Int32Value() : 0;
	long italic = args.Length() >= 4 ? args[3]->Int32Value() : 0;
	LOGFONTW logfont;
	ZeroMemory(&logfont, sizeof(logfont));
	logfont.lfHeight = size;
	logfont.lfWeight = weight;
	logfont.lfItalic = static_cast<BYTE>(italic);
	logfont.lfCharSet = DEFAULT_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	if( wcscpy_s(logfont.lfFaceName, LF_FACESIZE, (const wchar_t *)*fontName) != 0 ){
		Nan::ThrowTypeError("Too long font name");
		return;
	}
	HFONT font = CreateFontIndirectW(&logfont);
	args.GetReturnValue().Set(Nan::New((int)(UINT_PTR)font));
}

void deleteObject(const Nan::FunctionCallbackInfo<Value>& args){
	// deleteObject(obj)
	if( args.Length() < 1 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}	
	if( !args[0]->IsInt32() ){
		Nan::ThrowTypeError("wrong argument");
		return;
	}
	HANDLE object = (HANDLE)args[0]->Int32Value();
	BOOL ok = DeleteObject(object);
	args.GetReturnValue().Set(ok);
}

void getDpiOfHdc(const Nan::FunctionCallbackInfo<Value>& args){
	// getDpiOfHdc(hdc)
	if( args.Length() < 1 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}	
	if( !args[0]->IsInt32() ){
		Nan::ThrowTypeError("wrong arguments");
		return;
	}
	HDC hdc = (HDC)args[0]->Int32Value();
	int dpix = GetDeviceCaps(hdc, LOGPIXELSX);
	int dpiy = GetDeviceCaps(hdc, LOGPIXELSY);
	Local<Object> obj = Nan::New<v8::Object>();
	obj->Set(Nan::New("dpix").ToLocalChecked(), Nan::New(dpix));
	obj->Set(Nan::New("dpiy").ToLocalChecked(), Nan::New(dpiy));
	args.GetReturnValue().Set(obj);
}

static HANDLE alloc_handle(void *data, int len)
{
	HANDLE handle;
	void *ptr;

	handle = GlobalAlloc(GHND, len);
	ptr = GlobalLock(handle);
	memmove(ptr, data, len);
	GlobalUnlock(handle);
	return handle;
}

void printerDialog(const Nan::FunctionCallbackInfo<Value>& args){
	// printerDialog(devmode?, devnames?)
	HWND hwnd = create_window();
	if( hwnd == NULL ){
		Nan::ThrowTypeError("create_window failed");
		return;
	}
	DEVMODEW *devmodePtr = NULL;
	int devmodeLength = 0;
	DEVNAMES *devnamesPtr = NULL;
	int devnamesLength = 0;
	if( args.Length() >= 1 ){
		if( !args[0]->IsObject() ){
			Nan::ThrowTypeError("wrong arguments");
			return;
		}
		Local<Object> obj = args[0]->ToObject();
		devmodePtr = (DEVMODEW *)node::Buffer::Data(obj);
		devmodeLength = node::Buffer::Length(obj);
	}
	if( args.Length() >= 2 ){
		if( !args[1]->IsObject() ){
			Nan::ThrowTypeError("wrong arguments");
			return;
		}
		Local<Object> obj = args[1]->ToObject();
		devnamesPtr = (DEVNAMES *)node::Buffer::Data(obj);
		devnamesLength = node::Buffer::Length(obj);
	}
	PRINTDLGEXW pd;
	ZeroMemory(&pd, sizeof(pd));
	pd.lStructSize = sizeof(pd);
	pd.hwndOwner = hwnd;
	pd.Flags = PD_NOPAGENUMS;
	pd.nCopies = 1;
	pd.nStartPage = START_PAGE_GENERAL;
	if( devmodePtr ){
		pd.hDevMode = alloc_handle(devmodePtr, devmodeLength);
	}
	if( devnamesPtr ){
		pd.hDevNames = alloc_handle(devnamesPtr, devnamesLength);
	}
	HRESULT res = PrintDlgExW(&pd);
	dispose_window(hwnd);
	if( res == S_OK && pd.dwResultAction != PD_RESULT_CANCEL ){
		DEVMODEW *devmodePtr = (DEVMODEW *)GlobalLock(pd.hDevMode);
		int devmodeLength = sizeof(DEVMODEW) + devmodePtr->dmDriverExtra;
		Local<Object> devmodeBuffer = Nan::CopyBuffer((char *)devmodePtr, devmodeLength).ToLocalChecked();
		GlobalUnlock(pd.hDevMode);
		GlobalFree(pd.hDevMode);
		DEVNAMES *devnamesPtr = (DEVNAMES *)GlobalLock(pd.hDevNames);
		WCHAR *outputPtr = ((WCHAR *)devnamesPtr) + devnamesPtr->wOutputOffset;
		int outputLen = wcslen(outputPtr);
		int devnamesLength = (devnamesPtr->wOutputOffset + outputLen + 1) * 2;
		Local<Object> devnamesBuffer = Nan::CopyBuffer((char *)devnamesPtr, devnamesLength).ToLocalChecked();
		GlobalUnlock(pd.hDevNames);
		GlobalFree(pd.hDevNames);
		Local<Object> obj = Nan::New<v8::Object>();
		obj->Set(Nan::New("devmode").ToLocalChecked(), devmodeBuffer);
		obj->Set(Nan::New("devnames").ToLocalChecked(), devnamesBuffer);
		args.GetReturnValue().Set(obj);
	} else {
		args.GetReturnValue().Set(false);
	}
}

void parseDevmode(const Nan::FunctionCallbackInfo<Value>& args){
	// parseDevmode(devmode)
	if( args.Length() < 1 ){
		Nan::ThrowTypeError("wrong number of arguments");
		return;
	}	
	if( !args[0]->IsObject() ){
		Nan::ThrowTypeError("wrong arguments");
		return;
	}
	Local<Object> devmodeBuffer = args[0]->ToObject();
	DEVMODEW *devmodePtr = (DEVMODEW *)node::Buffer::Data(devmodeBuffer);
	DWORD fields = devmodePtr->dmFields;
	const uint16_t *cDevName = (const uint16_t *)devmodePtr->dmDeviceName;
	Local<String> deviceName = Nan::New(cDevName, lstrlenW((LPCWSTR)cDevName)).ToLocalChecked();
	Local<Object> obj = Nan::New<v8::Object>();
	obj->Set(Nan::New("deviceName").ToLocalChecked(), deviceName);
	obj->Set(Nan::New("orientation").ToLocalChecked(), Nan::New(devmodePtr->dmOrientation));
	obj->Set(Nan::New("paperSize").ToLocalChecked(), Nan::New(devmodePtr->dmPaperSize));
	obj->Set(Nan::New("copies").ToLocalChecked(), Nan::New(devmodePtr->dmCopies));
	obj->Set(Nan::New("printQuality").ToLocalChecked(), Nan::New(devmodePtr->dmPrintQuality));
	obj->Set(Nan::New("defaultSource").ToLocalChecked(), Nan::New(devmodePtr->dmDefaultSource));
	args.GetReturnValue().Set(obj);
}

void Init(v8::Local<v8::Object> exports){
	if( !initWindowClass() ){
		Nan::ThrowTypeError("initWindowClass failed");
		return;
	}
	exports->Set(Nan::New("createWindow").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(createWindow)->GetFunction());
	exports->Set(Nan::New("disposeWindow").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(disposeWindow)->GetFunction());
	exports->Set(Nan::New("getDc").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(getDc)->GetFunction());
	exports->Set(Nan::New("releaseDc").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(releaseDc)->GetFunction());
	exports->Set(Nan::New("measureText").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(measureText)->GetFunction());
	exports->Set(Nan::New("createFont").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(createFont)->GetFunction());
	exports->Set(Nan::New("deleteObject").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(deleteObject)->GetFunction());
	exports->Set(Nan::New("getDpiOfHdc").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(getDpiOfHdc)->GetFunction());
	exports->Set(Nan::New("printerDialog").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(printerDialog)->GetFunction());
	exports->Set(Nan::New("parseDevmode").ToLocalChecked(),
			Nan::New<v8::FunctionTemplate>(parseDevmode)->GetFunction());
}

NODE_MODULE(drawer, Init)


