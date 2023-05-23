
#include "main.h"


class DemoApp
{
public:
	DemoApp();
	~DemoApp(); //모든 자원을 반납
	HRESULT Initialize(HINSTANCE hInstance); //윈도우 생성, CreateAppResource() 호출
	HeyHo createHeyHo(Player p);
private:
	HRESULT CreateAppResource(); //장치 독립적 그리기 자원을 생성
	HRESULT CreateDeviceResource(); //장치 의존 자원을 생성
	void DiscardDeviceResource(); //장치 의존 자원을 반납
	void OnPaint(); //내용을 그리기
	void OnResize(); //렌더타겟을 resize
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //윈도우 프로시져
	void moveYoshi(char key);
	void stopYoshi();
	void setYoshiLoc();
	void setYoshiImg();
	void stopThrowing();

	void heyHoForOneFrame();
	void ThrwdEggForOneFrame();
	void yoshi_throw();
	Player getP();

	void eggAction();
	void write_loc();

private:
	//클래스의 변수들을 선언
	HWND hwnd;
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pLightSlateGrayBrush;
	ID2D1SolidColorBrush* pBlackBrush;

	IDWriteTextFormat* pTextFormat; //text
	IDWriteFactory* pDWriteFactory;

	IWICImagingFactory* pWICFactory; //image
	ID2D1Bitmap* pPlayerImage;
	ID2D1Bitmap* playerImages[8];
	ID2D1Bitmap* pHeyHoImage;
	ID2D1Bitmap* heyhoImages[6];
	ID2D1Bitmap* pEggImage;
	ID2D1Bitmap* pMapImage;
	ID2D1Bitmap* pKeyImage;
	ID2D1Bitmap* pScoreImage;

	Player yoshi;
	vector<HeyHo> heyhos;
	HeyHo heyho;
	Egg egg;
	vector<Thrwd_Egg> thrwd_eggs;
	Thrwd_Egg thrwd_egg;

	// 알 위치 계산용
	float _ploc[5][2];
	int egg_amount;


	int count;
	int frame_count;

	float block_point[2];

	int heyho_regen;
	int egg_regen;
	float egg_depth;






};


// 생성자. 멤버 변수들을 초기화함.
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
	heyho(),
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

	heyho_regen = 4000;
	egg_regen = 2000;
	egg_depth = 80;

}
// 소멸자. 응용 프로그램의 모든 자원을 반납함.
DemoApp::~DemoApp()
{
	// 장치 의존적 자원 반납
	DiscardDeviceResource();

	// 장치 독립적 자원 반납
	SAFE_RELEASE(pD2DFactory);

	SAFE_RELEASE(pTextFormat);  //text
	SAFE_RELEASE(pDWriteFactory);

	SAFE_RELEASE(pPlayerImage);//image
	SAFE_RELEASE(pWICFactory);

}
// 응용 프로그램의 원도우를 생성하고, 장치 독립적 자원을 생성함.
HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
	// 장치 독립적 자원을 생성함.
	HRESULT hr = CreateAppResource();
	if (FAILED(hr)) return hr;

	// 윈도우 클래스를 등록함..
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

	// 윈도우를 생성함.
	hwnd = CreateWindow(L"DemoApp", L"Yoshi minigame!", WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT , 1080, 720, NULL, NULL, hInstance, this);
	hr = hwnd ? S_OK : E_FAIL;
	if (!hwnd) return E_FAIL;

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);



	return hr;
}
// 장치 독립적 자원들을 생성함. 이들 자원의 수명은 응용 프로그램이 종료되기 전까지 유효함.
HRESULT DemoApp::CreateAppResource()
{

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(pDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory));

	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// D2D 팩토리를 생성함.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr)) return hr;

	// WIC 팩토리를 생성함.
	// 주의: WIC 팩토리를 생성하는 CoCreateInstance 함수가 사용될 때에는 이전에 CoInitialize를 호출해주어야 함.
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr)) return hr;

	return hr;
}
// 장치 의존적 자원들을 생성함. 장치가 소실되는 경우에는 이들 자원을 다시 생성해야 함.
HRESULT DemoApp::CreateDeviceResource()
{
	HRESULT hr = S_OK;

	if (pRenderTarget) return hr;

	RECT rc;
	GetClientRect(hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	// D2D 렌더타겟을 생성함.
	hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);
	if (FAILED(hr)) return hr;

	// 외부 파일로부터 비트맵 객체 pAnotherBitmap를 생성함.
	if (FAILED(hr)) return hr;

	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_r.png", 528, 720, &pPlayerImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh0.png", 245, 245, &pHeyHoImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\egg\\yoshi_egg.png", 256, 255, &pEggImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\realBackground.png", 1080,720, &pMapImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\key.png", 1383, 826, &pKeyImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\score.png", 1426, 697, &pScoreImage);


	//헤이호 이미지 전체 정의
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh0.png", 245, 245, &heyhoImages[0]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh1.png", 245, 245, &heyhoImages[1]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh2.png", 245, 245, &heyhoImages[2]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh0_l.png", 245, 245, &heyhoImages[3]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh1_l.png", 245, 245, &heyhoImages[4]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh2_l.png", 245, 245, &heyhoImages[5]);


	//요시 이미지 전체 정의
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_r.png", 245, 245, &playerImages[0]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_l.png", 245, 245, &playerImages[1]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run.png", 245, 245, &playerImages[2]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run2.png", 245, 245, &playerImages[3]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_throw.png", 245, 245, &playerImages[4]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run_l.png", 245, 245, &playerImages[5]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run2_l.png", 245, 245, &playerImages[6]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_throw_l.png", 245, 245, &playerImages[7]);


	//blackbrush 정의
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush);

	//yoshi정의
	yoshi = Player(pPlayerImage);
	//heyho 정의
	heyho = HeyHo(pHeyHoImage, yoshi.getX(),yoshi.getY());
	heyhos.push_back(heyho);
	//egg 정의
	egg = Egg(pEggImage);

	return hr;
}
// 장치 의존적 자원들을 반납함. 장치가 소실되면 이들 자원을 다시 생성해야 함.
void DemoApp::DiscardDeviceResource()
{
	SAFE_RELEASE(pRenderTarget);
	SAFE_RELEASE(pLightSlateGrayBrush);
	SAFE_RELEASE(pBlackBrush);
}
// 그릴 내용을 화면에 그림.
void DemoApp::OnPaint()
{
	//요시 움직임을 처리
	setYoshiLoc(); 

	HRESULT hr = CreateDeviceResource();
	if (FAILED(hr)) return;


	pRenderTarget->BeginDraw();
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

	// pBitmapFromFile을 그림.
	D2D1_SIZE_F size = pMapImage->GetSize();
	RECT rc;
	GetClientRect(hwnd, &rc);

	//맵 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0.0f, 0.0f));
	pRenderTarget->DrawBitmap(pMapImage, D2D1::RectF(0.0f, 0.0f, rc.right - rc.left, rc.bottom - rc.top), 0.84f);

	//알 그림
	float addsome = (yoshi.getDirection() == 'D') ? -30.0f : +50.0f; // 알 위치
	float addsome2 = (yoshi.getDirection() == 'D') ? -15.0f : +15.0f; //알 간격
	for (int i = 1; i <= egg_amount; i++) {
		int j = egg_amount - i;
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation( _ploc[j][0] + addsome + addsome2*(i-1), _ploc[j][1] + 60.0f));
		pRenderTarget->DrawBitmap(pEggImage, D2D1::RectF(0.0f, 0.0f, egg.getSize()[0], egg.getSize()[1]));
	}

	//요시 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(yoshi.getX(), yoshi.getY()));
	pRenderTarget->DrawBitmap(pPlayerImage, D2D1::RectF(0.0f, 0.0f,
		yoshi.isThrowing() ? yoshi.getSize()[0] + 14.0f: yoshi.getSize()[0],
		yoshi.getSize()[1]));

	//헤이호 컨트롤
	heyHoForOneFrame();

	//던져진 알 컨트롤
	ThrwdEggForOneFrame();


	//키 도움말 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(20.0f, 530.0f));
	pRenderTarget->DrawBitmap(pKeyImage, D2D1::RectF(0.0f, 0.0f, 213.0f , 133.0f ));
	//스코어 전광판 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f, 575.0f));
	pRenderTarget->DrawBitmap(pScoreImage, D2D1::RectF(0.0f,0.0f, 300.0f , 150.0f ));


	
	hr = pRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET)
	{
		DiscardDeviceResource();
	}


	
}
// 렌더타겟의 크기를 다시 설정함.
void DemoApp::OnResize()
{
	if (!pRenderTarget) return;

	RECT rc;
	GetClientRect(hwnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	pRenderTarget->Resize(size);
}

// 윈도우 메시지를 처리.
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		DemoApp* pDemoApp = (DemoApp*)pCreate->lpCreateParams;


		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pDemoApp);

		SetTimer(hwnd, 1, 1000 / 60, NULL); //60fps
		SetTimer(hwnd, 2, pDemoApp->heyho_regen, NULL);
		SetTimer(hwnd, 3, pDemoApp->egg_regen, NULL);
		SetTimer(hwnd, 4, pDemoApp->egg_depth, NULL);
		return 1;
	}

	DemoApp* pDemoApp = (DemoApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (!pDemoApp)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	/*
	pDemoApp->frame_count++;
	if (pDemoApp->frame_count > 3) { // >  뒤 숫자가 높아질수록 프레임 느림
		pDemoApp->OnPaint();
		pDemoApp->frame_count = 0;
	}*/

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
		if (wParam == VK_SPACE) {
			if (pDemoApp->egg_amount > 0) {
				pDemoApp->egg_amount--;
				pDemoApp->yoshi.ready();
				pDemoApp->stopYoshi();
			}
			break;
		}
		if (!pDemoApp->yoshi.isThrowing()) {

			if (wParam == 'W' || wParam == 'A' || wParam == 'S' || wParam == 'D') {

				pDemoApp->moveYoshi(wParam);
			}
		}

		//pDemoApp->OnPaint();
		return 0;
	}
	case WM_KEYUP:
	{
		if (pDemoApp->yoshi.isThrowing()&&wParam == VK_SPACE) {
			pDemoApp->yoshi_throw();

			break;
		}

		else if (!pDemoApp->yoshi.isThrowing()) {
			if (wParam == 'W' || wParam == 'A' || wParam == 'S' || wParam == 'D') {
				pDemoApp->stopYoshi();
			}
		}
		//pDemoApp->OnPaint();
		return 0;
	}
	case WM_TIMER:
	{
		if (wParam == 1) {
			pDemoApp->OnPaint();
		}
		else if (wParam == 2) {
			pDemoApp->heyhos.push_back(pDemoApp->createHeyHo(pDemoApp->getP()));

		}
		else if (wParam == 3) {
			pDemoApp->eggAction();
		}

		else if (wParam == 4) {
			pDemoApp->write_loc();
		}
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
// 응용 프로그램의 진입점 함수.
// 응용 프로그램의 진입점 함수.
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
	if (!yoshi.isThrowing()) {
		if (yoshi.getDir() == 'D') {
			pPlayerImage = playerImages[0];
		}
		else if (yoshi.getDir() == 'A') {
			pPlayerImage = playerImages[1];

		}
	}
	yoshi.setDir(' ');
}
void DemoApp::stopThrowing() {
	yoshi.setNImg('1');
	if (yoshi.getDirection() == 'D') {
		pPlayerImage = playerImages[0];
	}
	else if (yoshi.getDirection() == 'A') {
		pPlayerImage = playerImages[1];

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
	float speed = 8.5f;

	count++;
	if (count > 6) { // >  뒤 숫자가 높아질수록 이미지 변환 느림
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

	//윗 바닥 기울기 조정
	if (yoshi.getY() < 330.0f) {
		yoshi.setY(DEFAULT_Y_LOC_2F + (yoshi.getX() - BLOCK_L_2F) * 0.05f);
	}
	//아랫 바닥 기울기 조정
	else{
		yoshi.setY(DEFAULT_Y_LOC_1F - abs(yoshi.getX() - 500.0f) * 0.10f);
	}

}
void DemoApp::setYoshiImg() {

	if (yoshi.isThrowing()) {
		if (yoshi.getDirection() == 'D') {
			pPlayerImage = playerImages[4];

		}
		else {

			pPlayerImage = playerImages[7];
		}
		return;
	}
	char nowImg = yoshi.getNImg();

	if (nowImg == '1') {
		char c = yoshi.getDir();
		if (c == 'D') {
			yoshi.setNImg('2');
			pPlayerImage = playerImages[2];
		}
		else if (c == 'A') {
			yoshi.setNImg('4');
			pPlayerImage = playerImages[5];

		}
	}
	else if (nowImg == '2') {
		yoshi.setNImg('3');
		pPlayerImage = playerImages[3];
	}
	else if (nowImg == '3') {
		yoshi.setNImg('2');
		pPlayerImage = playerImages[2];
	}
	else if (nowImg == '4') {
		yoshi.setNImg('5');
		pPlayerImage = playerImages[6];
	}
	else if (nowImg == '5') {
		yoshi.setNImg('4');
		pPlayerImage = playerImages[5];
	}
}

Player DemoApp::getP() {
	return yoshi;
}
HeyHo DemoApp::createHeyHo(Player p) {
	return HeyHo( pHeyHoImage,p.getX(),p.getY());
}
void DemoApp::heyHoForOneFrame() {
	
	//헤이호 이미지 변환
	for ( int i = 0; i < heyhos.size(); i++) {

		int ni = heyhos[i].nextImg();
		if(heyhos[i].getDirectionX() > 0) {

			if (ni == 0) {
				pHeyHoImage = heyhoImages[0];
			}
			else  if (ni == 1) {
				pHeyHoImage = heyhoImages[1];
			}
			else {
				pHeyHoImage = heyhoImages[2];
			}
		}
		else {

			if (ni == 0) {
				pHeyHoImage = heyhoImages[3];
			}
			else  if (ni == 1) {
				pHeyHoImage = heyhoImages[4];
			}
			else {
				pHeyHoImage = heyhoImages[5];
			}
		}

		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation( heyhos[i].getX() , heyhos[i].getY()));
		pRenderTarget->DrawBitmap(pHeyHoImage, D2D1::RectF(0, 0, heyhos[i].getSize()[0], heyhos[i].getSize()[1]));

		//헤이호 이동
		heyhos[i].setX(heyhos[i].getX() + heyhos[i].getDirectionX() / heyhos[i].getSpd());
		heyhos[i].setY(heyhos[i].getY() + heyhos[i].getDirectionY() / heyhos[i].getSpd());

		if (heyhos[i].getX() < -50 || heyhos[i].getX() > 1130 || heyhos[i].getY() < -50 || heyhos[i].getY() > 770) {
			
			heyhos.erase(heyhos.begin() + i, heyhos.begin() + i + 1);
			
			i--;
		}

	}
}

void DemoApp::eggAction() {
	if (egg_amount < 3) {
		egg_amount++;
	}
}

void DemoApp::write_loc() {
	for (int i = 0; i < 4; i++) {
		_ploc[i][0] = _ploc[i+1][0];
		_ploc[i][1] = _ploc[i + 1][1];
	}
	_ploc[4][0] = yoshi.getX();
	_ploc[4][1] = yoshi.getY();
}

void DemoApp::ThrwdEggForOneFrame() {
	for (int i = 0; i < thrwd_eggs.size(); i++) {
		
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(thrwd_eggs[i].getX(), thrwd_eggs[i].getY()));
		pRenderTarget->DrawBitmap(pEggImage, D2D1::RectF(0, 0, thrwd_eggs[i].getSize()[0], thrwd_eggs[i].getSize()[1]));
		
		if (thrwd_eggs[i].getX() < -50 || thrwd_eggs[i].getX() > 1130 || thrwd_eggs[i].getY() < -50 || thrwd_eggs[i].getY() > 770) {

			thrwd_eggs.erase(thrwd_eggs.begin() + i, thrwd_eggs.begin() + i + 1);

			i--;
		}
		else {
			thrwd_eggs[i].next();

		}
		


	
	}
}

void DemoApp::yoshi_throw() {
	stopThrowing(); 
	thrwd_eggs.push_back(Thrwd_Egg(pEggImage, yoshi.getX(), yoshi.getY(), 20.0f, -7.0f, yoshi.getDirection() == 'D' ? +1.0f : -1.0f));
	yoshi.throw_egg();
	}