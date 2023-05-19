
#include "main.h"


class DemoApp
{
public:
	DemoApp();
	~DemoApp(); //��� �ڿ��� �ݳ�
	HRESULT Initialize(HINSTANCE hInstance); //������ ����, CreateAppResource() ȣ��
private:
	HRESULT CreateAppResource(); //��ġ ������ �׸��� �ڿ��� ����
	HRESULT CreateDeviceResource(); //��ġ ���� �ڿ��� ����
	void DiscardDeviceResource(); //��ġ ���� �ڿ��� �ݳ�
	void OnPaint(); //������ �׸���
	void OnResize(); //����Ÿ���� resize
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //������ ���ν���
	void moveYoshi(char key);
	void stopYoshi();
	void setYoshiLoc();
	void setYoshiImg();

private:
	//Ŭ������ �������� ����
	HWND hwnd;
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pLightSlateGrayBrush;
	ID2D1SolidColorBrush* pBlackBrush;

	IDWriteTextFormat* pTextFormat; //text
	IDWriteFactory* pDWriteFactory;

	IWICImagingFactory* pWICFactory; //image
	ID2D1Bitmap* pPlayerImage;
	ID2D1Bitmap* pHeyHoImage;
	ID2D1Bitmap* pMapImage;
	ID2D1Bitmap* pKeyImage;
	ID2D1Bitmap* pScoreImage;

	Player yoshi;
	vector<HeyHo> heyhos;

	int count;
	int frame_count;

	float block_point[2];







};


// ������. ��� �������� �ʱ�ȭ��.
DemoApp::DemoApp() :

	hwnd(NULL),
	pD2DFactory(NULL),
	pRenderTarget(NULL),
	pLightSlateGrayBrush(NULL),
	pBlackBrush(NULL),

	pDWriteFactory(NULL),  //text
	pTextFormat(NULL),

	pPlayerImage(NULL),
	pWICFactory(NULL),
	yoshi(),
	pMapImage(NULL)



{
	pDWriteFactory = NULL;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(pDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory));


	pTextFormat = NULL;	 //text
	pDWriteFactory->CreateTextFormat(
		L"Verdana",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		50,
		L"",
		&pTextFormat);

	count = 0;
	frame_count = 0;
	block_point[0] = BLOCK_L_1F;
	block_point[1] = BLOCK_R_1F;
}
// �Ҹ���. ���� ���α׷��� ��� �ڿ��� �ݳ���.
DemoApp::~DemoApp()
{
	// ��ġ ������ �ڿ� �ݳ�
	DiscardDeviceResource();

	// ��ġ ������ �ڿ� �ݳ�
	SAFE_RELEASE(pD2DFactory);

	SAFE_RELEASE(pTextFormat);  //text
	SAFE_RELEASE(pDWriteFactory);

	SAFE_RELEASE(pPlayerImage);//image
	SAFE_RELEASE(pWICFactory);

}
// ���� ���α׷��� �����츦 �����ϰ�, ��ġ ������ �ڿ��� ������.
HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
	// ��ġ ������ �ڿ��� ������.
	HRESULT hr = CreateAppResource();
	if (FAILED(hr)) return hr;

	// ������ Ŭ������ �����..
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DemoApp::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInstance;
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wcex.lpszClassName = L"DemoApp";
	RegisterClassEx(&wcex);

	// �����츦 ������.
	hwnd = CreateWindow(L"DemoApp", L"Yoshi minigame!", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT , 1080, 720, NULL, NULL, hInstance, this);
	hr = hwnd ? S_OK : E_FAIL;
	if (!hwnd) return E_FAIL;

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	return hr;
}
// ��ġ ������ �ڿ����� ������. �̵� �ڿ��� ������ ���� ���α׷��� ����Ǳ� ������ ��ȿ��.
HRESULT DemoApp::CreateAppResource()
{

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(pDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory));

	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// D2D ���丮�� ������.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr)) return hr;

	// WIC ���丮�� ������.
	// ����: WIC ���丮�� �����ϴ� CoCreateInstance �Լ��� ���� ������ ������ CoInitialize�� ȣ�����־�� ��.
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr)) return hr;

	return hr;
}
// ��ġ ������ �ڿ����� ������. ��ġ�� �ҽǵǴ� ��쿡�� �̵� �ڿ��� �ٽ� �����ؾ� ��.
HRESULT DemoApp::CreateDeviceResource()
{
	HRESULT hr = S_OK;

	if (pRenderTarget) return hr;

	RECT rc;
	GetClientRect(hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// D2D ����Ÿ���� ������.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);
	if (FAILED(hr)) return hr;

	// �ܺ� ���Ϸκ��� ��Ʈ�� ��ü pAnotherBitmap�� ������.
	if (FAILED(hr)) return hr;

	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_r.png", 300, 0, &pPlayerImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\heyho0.png", 245, 245, &pHeyHoImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\realBackground.png", 1080,720, &pMapImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\key.png", 1383, 826, &pKeyImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\score.png", 1426, 697, &pScoreImage);

	//yoshi����
	yoshi = Player(pPlayerImage);

	return hr;
}
// ��ġ ������ �ڿ����� �ݳ���. ��ġ�� �ҽǵǸ� �̵� �ڿ��� �ٽ� �����ؾ� ��.
void DemoApp::DiscardDeviceResource()
{
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pLightSlateGrayBrush);
	SAFE_RELEASE(pBlackBrush);
}
// �׸� ������ ȭ�鿡 �׸�.
void DemoApp::OnPaint()
{
	setYoshiLoc(); //1

	HRESULT hr = CreateDeviceResource();
	if (FAILED(hr)) return;


	pRenderTarget->BeginDraw();
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	// pBitmapFromFile�� �׸�.
	D2D1_SIZE_F size = pMapImage->GetSize();
	RECT rc;
	GetClientRect(hwnd, &rc);

	//�� �׸�
	pRenderTarget->DrawBitmap(pMapImage, D2D1::RectF(0.0f, 0.0f, rc.right - rc.left, rc.bottom - rc.top), 0.84f);
	

	//��� �׸�
	pRenderTarget->DrawBitmap(pPlayerImage, D2D1::RectF(yoshi.getX(), yoshi.getY(), yoshi.getSize()[0], yoshi.getSize()[1]));

	//Ű ���� �׸�
	pRenderTarget->DrawBitmap(pKeyImage, D2D1::RectF(20.0f, 530.0f, 213.0f + 20.0f, 133.0f + 530.0f));
	//���ھ� ������ �׸�
	pRenderTarget->DrawBitmap(pScoreImage, D2D1::RectF(740.0f, 520.0f, 300.0f + 740.0f, 150.0f + 520.0f));



	hr = pRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResource();
	}

	/* text

	static const WCHAR helloWorld[] = L"Hello, World!";

	pRenderTarget->DrawText(helloWorld, ARRAYSIZE(helloWorld) - 1,
		pTextFormat,
		D2D1::RectF(0, 0, size.width, size.height),
		pBlackBrush);
	*/
}
// ����Ÿ���� ũ�⸦ �ٽ� ������.
void DemoApp::OnResize()
{
	if (!pRenderTarget) return;

	RECT rc;
	GetClientRect(hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	pRenderTarget->Resize(size);
}

// ������ �޽����� ó��.
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		DemoApp* pDemoApp = (DemoApp*)pCreate->lpCreateParams;


		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDemoApp);

		return 1;
	}

	DemoApp* pDemoApp = (DemoApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!pDemoApp)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	/*
	pDemoApp->frame_count++;
	if (pDemoApp->frame_count > 3) { // >  �� ���ڰ� ���������� ������ ����
		pDemoApp->OnPaint();
		pDemoApp->frame_count = 0;
	}*/

	pDemoApp->OnPaint();
	switch (message)
	{
	case WM_SIZE:
	{
		pDemoApp->OnResize();
		return 0;
	}

	case WM_DISPLAYCHANGE:
	{
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	}

	case WM_PAINT:
	{
		pDemoApp->OnPaint();
		ValidateRect(hwnd, NULL);
		return 0;
	}

	case WM_KEYDOWN:
	{
		if (wParam == 'W' || wParam == 'A' || wParam == 'S' || wParam == 'D') {

			pDemoApp->moveYoshi(wParam);
		}
		pDemoApp->OnPaint();
		return 0;
	}
	case WM_KEYUP:
	{
		if (wParam == 'W' || wParam == 'A' || wParam == 'S' || wParam == 'D') {
			pDemoApp->stopYoshi();
		}
		pDemoApp->OnPaint();
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 1;
	}

	}


	return DefWindowProc(hwnd, message, wParam, lParam);
}
// ���� ���α׷��� ������ �Լ�.
// ���� ���α׷��� ������ �Լ�.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		DemoApp app;
		if (FAILED(app.Initialize(hInstance))) return 0;

		MSG msg = {};

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CoUninitialize();
	return 0;
}


// Creates a Direct2D bitmap from the specified file name.
HRESULT LoadBitmapFromFile(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(uri, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr)) return hr;

	// Create the initial frame.
	hr = pDecoder->GetFrame(0, &pSource);
	if (FAILED(hr)) return hr;

	// Convert the image format to 32bppPBGRA (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	hr = pIWICFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) return hr;

	// If a new width or height was specified, create an IWICBitmapScaler and use it to resize the image.
	if (destinationWidth != 0 || destinationHeight != 0)
	{
		UINT originalWidth, originalHeight;
		hr = pSource->GetSize(&originalWidth, &originalHeight);
		if (FAILED(hr)) return hr;

		if (destinationWidth == 0)
		{
			FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
			destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
		}
		else if (destinationHeight == 0)
		{
			FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
			destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
		}

		hr = pIWICFactory->CreateBitmapScaler(&pScaler);
		if (FAILED(hr)) return hr;

		hr = pScaler->Initialize(pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
		if (FAILED(hr)) return hr;

		hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr)) return hr;
	}
	else // Don't scale the image.
	{
		hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr)) return hr;
	}

	// Create a Direct2D bitmap from the WIC bitmap.
	hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
	if (FAILED(hr)) return hr;

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);

	return hr;
}

void DemoApp::moveYoshi(char key) {

	if (key == 'W') {
		if (yoshi.getY() > DEFAULT_Y_LOC_2F && yoshi.getX() > BLOCK_L_2F && yoshi.getX() < 680.0f) {
			yoshi.setY(yoshi.getY() - 185.0f);
			block_point[0] = BLOCK_L_2F;
			block_point[1] = 680.0f;
		}
	}
	else if (key == 'S') {
		if (yoshi.getY() < DEFAULT_Y_LOC_1F) {
			yoshi.setY(DEFAULT_Y_LOC_1F);
			block_point[0] = BLOCK_L_1F;
			block_point[1] = BLOCK_R_1F;
		}
	}
	else {
		yoshi.setDir(key);
	}
}
void DemoApp::stopYoshi() {
	yoshi.setNImg('1');
	if (yoshi.getDir() == 'D') {
		LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_r.png", 300, 0, &pPlayerImage);
	}
	else if (yoshi.getDir() == 'A') {
		LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_l.png", 300, 0, &pPlayerImage);

	}
	yoshi.setDir(' ');
}
void DemoApp::setYoshiLoc() {

	if (yoshi.getX() <= block_point[0]) {
		yoshi.setX(block_point[0]+1.0f);
		return;
	}
	else if (yoshi.getX() >= block_point[1]) {
		yoshi.setX(block_point[1]-1.0f);
	}
	char c = yoshi.getDir();
	float speed = 7.0f;

	count++;
	if (count > 6) { // >  �� ���ڰ� ���������� �̹��� ��ȯ ����
		setYoshiImg();
		count = 0;
	}

	if (c != ' ') {
		if (c == 'D') {
			yoshi.setX(yoshi.getX() + speed);
		}
		else if (c == 'A') {
			yoshi.setX(yoshi.getX() - speed);
		}
		else if (c == 'S') {
		}
	}

	//�� �ٴ� ���� ����
	if (yoshi.getY() < 330.0f) {
		yoshi.setY(DEFAULT_Y_LOC_2F + (yoshi.getX() - BLOCK_L_2F) * 0.05f);
	}
	//�Ʒ� �ٴ� ���� ����
	else{
		yoshi.setY(DEFAULT_Y_LOC_1F - abs(yoshi.getX() - 500.0f) * 0.10f);
	}

}
void DemoApp::setYoshiImg() {

	char nowImg = yoshi.getNImg();
	if (nowImg == '1') {
		char c = yoshi.getDir();
		if (c == 'D') {
			yoshi.setNImg('2');  LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run.png", 300, 0, &pPlayerImage);
		}
		else if (c == 'A') {
			yoshi.setNImg('4');  LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run_l.png", 300, 0, &pPlayerImage);

		}
	}
	else if (nowImg == '2') {
		yoshi.setNImg('3');  LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run2.png", 300, 0, &pPlayerImage);
	}
	else if (nowImg == '3') {
		yoshi.setNImg('2');  LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run.png", 300, 0, &pPlayerImage);
	}
	else if (nowImg == '4') {
		yoshi.setNImg('5');  LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run2_l.png", 300, 0, &pPlayerImage);
	}
	else if (nowImg == '5') {
		yoshi.setNImg('4');  LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run_l.png", 300, 0, &pPlayerImage);
	}
}
