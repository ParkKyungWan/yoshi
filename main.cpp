
#include "main.h"
#include "DirectSoundHelper.h"


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
	void CreateAppResource2(); //장치 의존 자원을 생성 - 중에서 음악만 뺌
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

	void power_targeting();
	void draw_dead_heyho();
	void YoshiStateController(); 



private:
	//클래스의 변수들을 선언
	HWND hwnd;
	ID2D1Factory* pD2DFactory;
	ID2D1HwndRenderTarget* pRenderTarget;
	ID2D1SolidColorBrush* pLightSlateGrayBrush;
	ID2D1SolidColorBrush* pBlackBrush;
	ID2D1SolidColorBrush* pRedbrush;

	IDWriteTextFormat* pTextFormat; //text
	IDWriteFactory* pDWriteFactory;

	IWICImagingFactory* pWICFactory; //image
	ID2D1Bitmap* pPlayerImage;
	ID2D1Bitmap* playerImages[8];
	ID2D1Bitmap* pHeyHoImage;
	ID2D1Bitmap* heyhoImages[8];
	ID2D1Bitmap* pEggImage;
	ID2D1Bitmap* pMapImage;
	ID2D1Bitmap* pKeyImage;
	ID2D1Bitmap* pScoreImage;
	ID2D1Bitmap* pTargetImageR;
	ID2D1Bitmap* pTargetImageL;

	ID2D1Bitmap* blackwords[10];
	ID2D1Bitmap* whitewords[10];

	Player yoshi;
	vector<HeyHo> heyhos;
	vector<dead_point> heyho_d_points;
	HeyHo heyho;
	Egg egg;
	vector<Thrwd_Egg> thrwd_eggs;
	Thrwd_Egg thrwd_egg;

	//사운드
	CSoundManager* soundManager;

	// 알 위치 계산용
	float _ploc[5][2];
	int egg_amount;
	int max_egg;


	int count;
	int frame_count;
	int count_for_level;

	float block_point[2];

	int heyho_regen;
	int egg_regen;
	float egg_depth;

	float power;
	float max_power;
	float power_dir;

	int hp;
	int score;
	int score_point;//스코어 정산 기준

	int damage_delay; // 1 = 1/60 초






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
	pMapImage(NULL),
	soundManager(NULL)



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
	egg_regen = 3000;
	egg_depth = 80;
	max_egg = 3;

	power = 0.0f;
	max_power = 15.f;
	power_dir = 1.0f;
	hp = 10;
	score = 0;
	score_point = 2;
	
	damage_delay = 60; //2초
	count_for_level = 0;

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

	SAFE_DELETE(soundManager);

}
// 응용 프로그램의 원도우를 생성하고, 장치 독립적 자원을 생성함.
HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{

	HRESULT hr = CreateAppResource();
	if (FAILED(hr))
		return TRACE(TEXT("CreateAppResource"));


	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DemoApp::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = hInstance;
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = L"DemoApp";
	RegisterClassEx(&wcex);

	// 윈도우를 생성함.
	hwnd = CreateWindow(L"DemoApp", L"Yoshi minigame!", (WS_OVERLAPPED | WS_SYSMENU), CW_USEDEFAULT, CW_USEDEFAULT , 1080, 720, NULL, NULL, hInstance, this);
	hr = hwnd ? S_OK : E_FAIL;
	if (!hwnd) return E_FAIL;


	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	// 윈도우가 초기화된 뒤에 hwnd가 유효한 값을 가진 후에 수행하자.
	// 이유: CSoundManager::Initialize() 에서 유효한 hwnd 값이 필요함.
	CreateAppResource2();

	return hr;
}



// 장치 독립적 자원들을 생성함. 이들 자원의 수명은 응용 프로그램이 종료되기 전까지 유효함.
HRESULT DemoApp::CreateAppResource()
{
	// D2D 팩토리를 생성함.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	if (FAILED(hr)) return hr;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr))
		return TRACE(TEXT("DWriteCreateFactory"));

	hr = pDWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.f, L"", &pTextFormat);
	if (FAILED(hr))
		return TRACE(TEXT("CreateTextFormat"));
	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// WIC 팩토리를 생성함.
	// 주의: WIC 팩토리를 생성하는 CoCreateInstance 함수가 사용될 때에는 이전에 CoInitialize를 호출해주어야 함.
	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
	if (FAILED(hr)) return hr;



	
	

	return hr;
}

void DemoApp::CreateAppResource2() {
	//사운드 설정
	
	soundManager = new CSoundManager;
	soundManager->init(hwnd);

	int id;  // 사운드클립 등록번호, id=0부터 시작함.
	soundManager->add(const_cast<LPTSTR>(L"levelup.wav"), &id); //id=0
	soundManager->add(const_cast<LPTSTR>(L"main120bpm.wav"), &id); //id=1
	soundManager->add(const_cast<LPTSTR>(L"main130bpm.wav"), &id); //id=2
	soundManager->add(const_cast<LPTSTR>(L"main140bpm.wav"), &id); //id=3
	soundManager->add(const_cast<LPTSTR>(L"main150bpm.wav"), &id); //id=4
	soundManager->add(const_cast<LPTSTR>(L"main160bpm.wav"), &id); //id=5
	soundManager->add(const_cast<LPTSTR>(L"hyick.wav"), &id); //id=6
	soundManager->add(const_cast<LPTSTR>(L"movejung.wav"), &id); //id=7
	soundManager->add(const_cast<LPTSTR>(L"ppyong.wav"), &id); //id=8
	soundManager->add(const_cast<LPTSTR>(L"profsmallsound.wav"), &id); //id=9
	soundManager->add(const_cast<LPTSTR>(L"puck.wav"), &id); //id=10
	soundManager->add(const_cast<LPTSTR>(L"yoshi-yap.wav"), &id); //id=11
	soundManager->add(const_cast<LPTSTR>(L"yoshi-hmmph.wav"), &id); //id=12
	soundManager->add(const_cast<LPTSTR>(L"afterpuck.wav"), &id); //id=13
	soundManager->add(const_cast<LPTSTR>(L"yoshi-ow.wav"), &id); //id=14
	soundManager->play(8, FALSE);
	soundManager->play(1, TRUE);

	
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
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\realBackground.png", 1080, 720, &pMapImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\key.png", 1383, 826, &pKeyImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\score.png", 1426, 697, &pScoreImage);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\target.png", 907, 692, &pTargetImageR);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\map\\target_l.png", 907, 692, &pTargetImageL);


	//헤이호 이미지 전체 정의
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh0.png", 245, 245, &heyhoImages[0]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh1.png", 245, 245, &heyhoImages[1]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh2.png", 245, 245, &heyhoImages[2]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh0_l.png", 245, 245, &heyhoImages[3]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh1_l.png", 245, 245, &heyhoImages[4]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh2_l.png", 245, 245, &heyhoImages[5]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh_down.png", 245, 245, &heyhoImages[6]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\monster\\hh_down_l.png", 245, 245, &heyhoImages[7]);


	//요시 이미지 전체 정의
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_r.png", 245, 245, &playerImages[0]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_l.png", 245, 245, &playerImages[1]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run.png", 245, 245, &playerImages[2]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run2.png", 245, 245, &playerImages[3]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_throw.png", 245, 245, &playerImages[4]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run_l.png", 245, 245, &playerImages[5]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_run2_l.png", 245, 245, &playerImages[6]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\images\\yoshi\\yoshi_throw_l.png", 245, 245, &playerImages[7]);

	//글자 이미지 전체 정의

	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\0.png", 426, 522, &blackwords[0]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\1.png", 426, 522, &blackwords[1]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\2.png", 426, 522, &blackwords[2]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\3.png", 426, 522, &blackwords[3]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\4.png", 426, 522, &blackwords[4]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\5.png", 426, 522, &blackwords[5]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\6.png", 426, 522, &blackwords[6]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\7.png", 426, 522, &blackwords[7]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\8.png", 426, 522, &blackwords[8]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\9.png", 426, 522, &blackwords[9]);


	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\0_w.png", 426, 522, &whitewords[0]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\1_w.png", 426, 522, &whitewords[1]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\2_w.png", 426, 522, &whitewords[2]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\3_w.png", 426, 522, &whitewords[3]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\4_w.png", 426, 522, &whitewords[4]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\5_w.png", 426, 522, &whitewords[5]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\6_w.png", 426, 522, &whitewords[6]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\7_w.png", 426, 522, &whitewords[7]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\8_w.png", 426, 522, &whitewords[8]);
	LoadBitmapFromFile(pRenderTarget, pWICFactory, L".\\fonts\\9_w.png", 426, 522, &whitewords[9]);


	//blackbrush 정의a
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush);
	//redbrush 정의
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pRedbrush);

	//yoshi정의
	yoshi = Player(pPlayerImage);
	//heyho 정의
	heyho = HeyHo(pHeyHoImage, yoshi.getX(), yoshi.getY());
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
	//요시의 변형된 상태를 처리
	YoshiStateController();

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

	if (yoshi.isThrowing()) {
		power_targeting();
	}

	//요시 그림
	float player_opacity = 1.0f;
	if (yoshi.getState() == 1) {
		player_opacity = 0.6f;
		float sumsum = (yoshi.getAC() < 15) ? ((yoshi.getAC() % 3) * 3.0f) : 0.0f; //(15프레임간) 떨림
		pRenderTarget->SetTransform(
			D2D1::Matrix3x2F::Translation(yoshi.getX(), yoshi.getY() - sumsum)
		);
		pRenderTarget->DrawBitmap(pPlayerImage, D2D1::RectF(0.0f, 0.0f,
			yoshi.isThrowing() ? yoshi.getSize()[0] + 14.0f : yoshi.getSize()[0],
			yoshi.getSize()[1]), player_opacity);
	}
	else if (yoshi.getState() == 0) {
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(yoshi.getX(), yoshi.getY()));
		pRenderTarget->DrawBitmap(pPlayerImage, D2D1::RectF(0.0f, 0.0f,
			yoshi.isThrowing() ? yoshi.getSize()[0] + 14.0f : yoshi.getSize()[0],
			yoshi.getSize()[1]), player_opacity);
	}

	//던져진 알 컨트롤
	ThrwdEggForOneFrame();

	//헤이호 컨트롤
	heyHoForOneFrame();
	draw_dead_heyho();


	//키 도움말 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(20.0f, 530.0f));
	pRenderTarget->DrawBitmap(pKeyImage, D2D1::RectF(0.0f, 0.0f, 213.0f , 133.0f ));
	
	//스코어 전광판 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f, 575.0f));
	pRenderTarget->DrawBitmap(pScoreImage, D2D1::RectF(0.0f,0.0f, 300.0f , 150.0f ));
	
	//hp바 그림
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f +85.0f , 575.0f + 15.0f));
	pRenderTarget->FillRectangle(D2D1::RectF(0.0f, 0.0f, (200.0f/10.0f)*hp, 20.0f), pRedbrush);

	//알이 몇개
	ID2D1Bitmap* tmpbit = whitewords[egg_amount];
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f + 250.0f, 575.0f + 45.0f));
	pRenderTarget->DrawBitmap(tmpbit, D2D1::RectF(0.0f, 0.0f, 42.0f, 52.0f));

	//몇점
	tmpbit = whitewords[score%10];
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f + 157.0f, 575.0f + 45.0f));
	pRenderTarget->DrawBitmap(tmpbit, D2D1::RectF(0.0f, 0.0f, 42.0f, 52.0f));

	tmpbit = whitewords[(score%100-score%10)/10];
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f + 142.0f, 575.0f + 45.0f));
	pRenderTarget->DrawBitmap(tmpbit, D2D1::RectF(0.0f, 0.0f, 42.0f, 52.0f));

	tmpbit = whitewords[(score-score%100)/100];
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(740.0f + 127.0f, 575.0f + 45.0f));
	pRenderTarget->DrawBitmap(tmpbit, D2D1::RectF(0.0f, 0.0f, 42.0f, 52.0f));


	
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
			if (pDemoApp->egg_amount > 0&& !pDemoApp->yoshi.isThrowing()) {
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
		soundManager->play(8,FALSE);
		if (yoshi.getY() > DEFAULT_Y_LOC_2F && yoshi.getX() > BLOCK_L_2F && yoshi.getX() < 680.0f) {
			yoshi.setY(yoshi.getY() - 185.0f);
			block_point[0] = BLOCK_L_2F;
			block_point[1] = 680.0f;
		}
	}
	else if (key == 'S') {
		soundManager->play(8, FALSE);
		if (yoshi.getY() < DEFAULT_Y_LOC_1F) {
			yoshi.setY(DEFAULT_Y_LOC_1F);
			block_point[0] = BLOCK_L_1F;
			block_point[1] = BLOCK_R_1F;
		}
	}
	else {
		soundManager->play(7, FALSE);
		yoshi.setDir(key);
	}
}
void DemoApp::stopYoshi() {
	soundManager->stop(7);
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


	//헤이호와 부딪혔을 때
	
	for (int i = 0; i < heyhos.size(); i++) {
		HeyHo* hh = &heyhos[i];

		if (yoshi.getState() != 1) {
			float yudori = 40.0f; //유도리 있게 좀 덜맞게 설정
			if (hh->isDamaged(yoshi.getX()+yudori, yoshi.getY() +yudori) || hh->isDamaged(yoshi.getX()+ yoshi.getSize()[0] - yudori, yoshi.getY()+ yudori) || hh->isDamaged(yoshi.getX()+ yudori, yoshi.getY() + yoshi.getSize()[1]- yudori) || hh->isDamaged(yoshi.getX() + yoshi.getSize()[0] - yudori, yoshi.getY() + yoshi.getSize()[1]- yudori)) {
				soundManager->play(14, FALSE);
				yoshi.setState(1);
				hp--;
				break;
			}
		}
		
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
	if (heyhos.size() == 0|| heyhos.size() == 1) {
		soundManager->play(9, TRUE);
	}
	return HeyHo( pHeyHoImage,p.getX(),p.getY());
}
void DemoApp::heyHoForOneFrame() {

	if (heyhos.size() == 0) {
		soundManager->stop(9);
	}
	
	//헤이호 이미지 변환
	for ( int i = 0; i < heyhos.size(); i++) {

		//알 맞았는지 확인
		bool check_damaged = false;
		for (int j = 0; j < thrwd_eggs.size(); j++) {
			float tmp_x = thrwd_eggs[j].getX();
			float tmp_y = thrwd_eggs[j].getY();
			
			if (heyhos[i].isDamaged(tmp_x, tmp_y)) { //알 맞음!
				soundManager->play(10, FALSE);
				soundManager->play(13, FALSE);
				score += 2;
				dead_point dp = { (thrwd_eggs[j].getps()* thrwd_eggs[j].getDir()) /1.5f,(thrwd_eggs[j].getpd()) / 1.5f, heyhos[i].getX() ,heyhos[i].getY(), 0};
				heyho_d_points.push_back(dp);
				thrwd_eggs.erase(thrwd_eggs.begin() + j, thrwd_eggs.begin() + j + 1);
				heyhos.erase(heyhos.begin() + i, heyhos.begin() + i + 1);
				i--;
				check_damaged = true;
			}
		}
		if (check_damaged) continue; //죽으면 다음 헤이호로 이동


		//헤이호 정상 작업 수행
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

		if (heyhos[i].getX() < -50 || heyhos[i].getX() > 1130 || heyhos[i].getY() < -50 || heyhos[i].getY() > 770) { //헤이호가 맵 밖으로 넘어감
			
			heyhos.erase(heyhos.begin() + i, heyhos.begin() + i + 1);
			score = (score - score_point * 2 < 0) ? 0 : score - score_point * 2; //헤이호를 놓치면 score 깎임
			i--;
		}

	}
}

void DemoApp::eggAction() {
	if (egg_amount < max_egg ) {
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
	soundManager->stop(12);
	soundManager->play(11, FALSE);
	soundManager->play(6, FALSE);
	stopThrowing(); 
	thrwd_eggs.push_back(Thrwd_Egg(pEggImage, yoshi.getX(), yoshi.getY(), 15.0f, ((yoshi.getDirection()=='A')? - 1.0f:1.0f) * power - 6.0f, yoshi.getDirection() == 'D' ? +1.0f : -1.0f));
	yoshi.throw_egg();
	}

void DemoApp::power_targeting() {

	soundManager->play(12, FALSE);
	power += 1.0f * power_dir;
	if (power > max_power || power < -1.0f * max_power) {
		power_dir *= -1.0f;
	}

	if( yoshi.getDirection() == 'D') {
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(power * 3.0f+22.5f,
			D2D1::Point2F(0.0f, 100.0f)) * D2D1::Matrix3x2F::Translation(yoshi.getX()+75.0f, yoshi.getY()-55.0f));
		pRenderTarget->DrawBitmap(pTargetImageR, D2D1::RectF(0.0f, 0.0f, 130.0f, 100.0f)); 
	}
	else {
		pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(power * 3.0f-22.5f,
			D2D1::Point2F(130.0f, 100.0f)) * D2D1::Matrix3x2F::Translation(yoshi.getX()-125.0f, yoshi.getY() - 50.0f));
		pRenderTarget->DrawBitmap(pTargetImageL, D2D1::RectF(0.0f, 0.0f, 130.0f, 100.0f));
	}
	
}

void DemoApp::draw_dead_heyho() {
	for (int i = 0; i < heyho_d_points.size(); i++) {
		dead_point* dp = &heyho_d_points[i];
		dp->count += 1;


		if (dp->x < -50 || dp->x > 1130 || dp->y < -50 || dp->y > 770) {
			heyho_d_points.erase(heyho_d_points.begin() + i, heyho_d_points.begin() + i + 1);
			i--; //맵 탈출 시 삭제
		}
		else{
			dp->x += dp->dirx ;
			dp->y += dp->diry ;
			pRenderTarget->SetTransform(
				D2D1::Matrix3x2F::Rotation( (dp->count+1.0f) * 20.0f * ((dp->dirx) / abs(dp->dirx)), D2D1::Point2F(heyho.getSize()[0] / 2.0f, heyho.getSize()[1]/2.0f))
				* D2D1::Matrix3x2F::Translation(dp->x, dp->y)); //360도 회전하며 날아감

			pRenderTarget->DrawBitmap( ((dp->dirx < 0.0f )? heyhoImages[6] : heyhoImages[7]), D2D1::RectF(0.0f, 0.0f, (heyho.getSize()[0]-5.0f), (heyho.getSize()[1]-10.0f)));
		}
	}
}
void DemoApp::YoshiStateController() {
	if (yoshi.getState() == 1) {
		yoshi.setAC(yoshi.getAC() + 1);
		if (yoshi.getAC() > damage_delay) {
			yoshi.setState(0);
			yoshi.setAC(0);
			return;
		}
	}
}