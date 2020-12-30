#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "Vector.h"
#include "Matrix.h"
#include <time.h>


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT(".");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
    , LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASS WndClass;
    g_hInst = hInstance;

    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = WndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = NULL;
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&WndClass);

    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}

#define SIZE 2048
#define PIE 3.1415 // 원주율

#define RADIUS 25 // 플레이어 반지름

#define ENEMY_RADIUS 10 // 몬스터 반지름
#define ENEMY_SPEED 1.0 // 몬스터 속도
#define ENEMY_MAXCOUNT 100 // 몬스터 객체 최대 개수
#define WAY_POINT_COUNT 5 // WayPoint 개수
#define ALERTVALUE 100 // 몬스터가 플레이어를 반응하는 거리

int EnemyCount;

struct CircleCollider
{
    Vec3D Center;
    float Radius;
    bool coll;
};

struct triangle
{
    Vec3D vertex[3];
    Vec3D Center;
    CircleCollider Circoll;
};

struct Monster
{
    triangle Enemy_t;
    Vec3D Way_Point[WAY_POINT_COUNT];
    int Point_Count;
};

bool ChangeColor = false;

/* 함수 원형 */
void Equation(float x1, float y1, float x2, float y2, float* a, float* b);
void DrawPrimitive(HDC hdc, triangle& t);
void DrawLine(HDC hdc, POINT* Line, float a, float b, float x1, float x2);
void MoveTriangle(triangle& t, float p_x, float p_y);
void CopyPack(triangle& t, triangle& Current_t);
void TranslateMatrix(Matrix4x4& p_mat, float p_dx, float p_dy, float p_dz);
void MatrixScaling(Matrix4x4& p_mat, float scaleX, float scaleY, float scaleZ);
void MatrixRotateX(Matrix4x4& p_mat, float angle);
void MatrixRotateY(Matrix4x4& p_mat, float angle);
void MatrixRotateZ(Matrix4x4& p_mat, float angle);
double Watching(int x1, int y1, int x2, int y2);
BOOL Alert(int x, int y, int mx, int my, const int radius);
void MoveEnemy(Monster* p_mob, triangle* p_cur_t, Matrix4x4& p_transmat);
Matrix4x4 EnemyMoveDirection(Vec3D p_x1, Vec3D p_x2, float a);
void MonsterManager(HDC p_backdc, Monster* p_monster, triangle* p_cur_t);
CircleCollider GetCircle(triangle p_t);
void DrawCircle(HDC backdc, CircleCollider p_coll);
float Degree_To_Radian(float angle);
bool CircleCollision(CircleCollider p_player, CircleCollider p_monster);

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc, backdc;
    static HBITMAP BackBitmap, OldBackBitmap;
    PAINTSTRUCT ps;

    static RECT rt;
    static POINT Line[SIZE]; // 직선의 방정식에 사용할 변수
    
    static triangle Player_t; // 플레이어
    static triangle Current_Player_t;

    static Matrix4x4 EnemyTransMat; // 몬스터 이동 행렬
    static Monster Monster[ENEMY_MAXCOUNT]; // 몬스터
    static triangle Current_Enemy_t[ENEMY_MAXCOUNT];

    static float MouseX, MouseY; // 마우스 클릭 좌표
    static bool Clickflag; // 마우스 클릭 플래그

    static Matrix4x4 TransMat; // 이동 행렬
    static Matrix4x4 OriginMat; // 원점으로 이동하는 행렬
    static Matrix4x4 ResultMat; // 행렬 병합 결과
    static float transVal, rotateVal, scaleVal;

    static float a, b; // 기울기, 절편

    //static TCHAR text1[256], text2[256], text3[256], text4[256]; // 좌표값 확인용 
   
    switch (iMessage) {

        /* 변수 초기값 설정 */
    case WM_CREATE:
        GetClientRect(hWnd, &rt); // 클라이언트 크기 (1424,720)
        srand(time(NULL));

        transVal = 30.0f; // 이동 값
        rotateVal = 10.0f; // 회전 값
        scaleVal = 1.1f; // 크기 증가 값

        /* 삼각형 중심 좌표 초기값 설정 */
        Player_t.Center = Vec3D(rt.right / 2, rt.bottom / 2, 1.0f);
        
        /* 삼각형 세 점의 좌표 초기값 설정 */
        Player_t.vertex[0] = Vec3D(Player_t.Center.Get_X(), Player_t.Center.Get_Y() - RADIUS, 1.0f);
        Player_t.vertex[1] = Vec3D(Player_t.Center.Get_X() - RADIUS, Player_t.Center.Get_Y() + RADIUS, 1.0f);
        Player_t.vertex[2] = Vec3D(Player_t.Center.Get_X() + RADIUS, Player_t.Center.Get_Y() + RADIUS, 1.0f);
        
        Player_t.Circoll = GetCircle(Player_t);
        CopyPack(Player_t, Current_Player_t); // 설정한 좌표 복사

        /* WayPoint 생성 */
        for (int i = 0; i < ENEMY_MAXCOUNT; i++)
        {
            for (int j = 0; j < WAY_POINT_COUNT; j++)
            {
                // 클라이언트 크기 안에 랜덤 위치에 포인트 저장
                Monster[i].Way_Point[j] = Vec3D(rand() % (rt.right - (RADIUS * 2)) + (rt.left + RADIUS), rand() % (rt.bottom - (RADIUS * 2)) + (rt.top + RADIUS), 1.0f);
            }
        }

        for (int i = 0; i < ENEMY_MAXCOUNT; i++)
        {
            /* 몬스터(삼각형) 생성 위치 . 첫번째 포인트에 생성 */
            Monster[i].Enemy_t.Center = Vec3D(Monster[i].Way_Point[0].Get_X(), Monster[i].Way_Point[0].Get_Y(), 1.0f);
            
            /* 몬스터(삼각형) 크기 설정 */
            Monster[i].Enemy_t.vertex[0] = Vec3D(Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y() - ENEMY_RADIUS, 1.0f);
            Monster[i].Enemy_t.vertex[1] = Vec3D(Monster[i].Enemy_t.Center.Get_X() - ENEMY_RADIUS, Monster[i].Enemy_t.Center.Get_Y() + ENEMY_RADIUS, 1.0f);
            Monster[i].Enemy_t.vertex[2] = Vec3D(Monster[i].Enemy_t.Center.Get_X() + ENEMY_RADIUS, Monster[i].Enemy_t.Center.Get_Y() + ENEMY_RADIUS, 1.0f);

            Monster[i].Point_Count = 0;
            Monster[i].Enemy_t.Circoll = GetCircle(Monster[i].Enemy_t);

            CopyPack(Monster[i].Enemy_t, Current_Enemy_t[i]);
        }

        Clickflag = false; // 마우스 클릭 플래그

        SetTimer(hWnd, 1, 10, NULL); 
        break;

        /* 마우스 클릭 */
    case WM_LBUTTONDOWN:
        /*MouseX = LOWORD(lParam);
        MouseY = HIWORD(lParam);

        Clickflag = true;*/
        
        break;

        /* 타이머 */
    case WM_TIMER:
        switch (wParam)
        {
        case 1:
            
            for (int i = 0; i < EnemyCount; i++)
            {
                /* 플레이어와 몬스터 근접(Follow) */
                if (Alert(Player_t.Center.Get_X(), Player_t.Center.Get_Y(),
                    Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(), ALERTVALUE)) // 플레이어 중심좌표, 몬스터 중심좌표
                {
                    /* 플레이어의 중심좌표와 몬스터 중심좌표의 기울기 */
                    Equation(Player_t.Center.Get_X(), Player_t.Center.Get_Y(),
                        Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(), &a, &b);

                    EnemyTransMat = EnemyMoveDirection(Monster[i].Enemy_t.Center, Player_t.Center, a);

                    MoveEnemy(&Monster[i], &Current_Enemy_t[i], EnemyTransMat);

                    /* 콜라이더 충돌 체크 */
                    if (CircleCollision(Player_t.Circoll, Monster[i].Enemy_t.Circoll))
                    {
                        Monster[i].Enemy_t.Circoll.coll = true;
                    }
                    else
                    {
                        Monster[i].Enemy_t.Circoll.coll = false;
                    }
                }

                /* 몬스터가 WayPoint에 근접(다음 WayPoint로 이동) */
                else if (Alert(Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(),
                    Monster[i].Way_Point[Monster[i].Point_Count].Get_X(), Monster[i].Way_Point[Monster[i].Point_Count].Get_Y(), ENEMY_RADIUS)) // 몬스터 중심좌표, WayPoint 좌표
                {
                    
                    ++Monster[i].Point_Count;

                    /* 마지막 포인트에 도착하면 카운트 초기화(마지막 포인트 -> 첫번째 포인트로 이동) */
                    if (Monster[i].Point_Count == WAY_POINT_COUNT)
                    {
                        Monster[i].Point_Count = 0;
                    }
                }

                else
                {
                    /* 몬스터의 중심좌표와 WayPoint좌표의 기울기 */
                    Equation(Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(),
                        Monster[i].Way_Point[Monster[i].Point_Count].Get_X(), Monster[i].Way_Point[Monster[i].Point_Count].Get_Y(), &a, &b);

                    EnemyTransMat = EnemyMoveDirection(Monster[i].Enemy_t.Center, Monster[i].Way_Point[Monster[i].Point_Count], a);

                    MoveEnemy(&Monster[i], &Current_Enemy_t[i], EnemyTransMat);

                    /* 콜라이더 충돌 체크(Follow 없을 때 확인용) */
                    if (CircleCollision(Player_t.Circoll, Monster[i].Enemy_t.Circoll))
                    {
                        Monster[i].Enemy_t.Circoll.coll = true;
                    }
                    else
                    {
                        Monster[i].Enemy_t.Circoll.coll = false;
                    }
                }
            }
                     
            break;
        }  
        InvalidateRect(hWnd, NULL, FALSE);
        break;

        /* 키 입력 */
    case WM_KEYDOWN:
        switch (wParam)
        {
            //https://shaeod.tistory.com/388 키 코드

            // W
        case 0x57:
            TranslateMatrix(TransMat, 0.0f, -transVal, 0.0f); // 이동행렬 값 설정
            Player_t.Center = TransMat.MatrixMultple1x4x4x4(Player_t.Center); // 중심점 이동
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = TransMat.MatrixMultple1x4x4x4(Player_t.vertex[i]); // 세 점 이동
            }
            Player_t.Circoll = GetCircle(Player_t);
            CopyPack(Player_t, Current_Player_t);
            break;

            // S
        case 0x53:
            TranslateMatrix(TransMat, 0.0f, transVal, 0.0f);
            Player_t.Center = TransMat.MatrixMultple1x4x4x4(Player_t.Center);
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = TransMat.MatrixMultple1x4x4x4(Player_t.vertex[i]);
            }
            Player_t.Circoll = GetCircle(Player_t);
            CopyPack(Player_t, Current_Player_t);
            break;

            // A
        case 0x41:
            TranslateMatrix(TransMat, -transVal, 0.0f, 0.0f);
            Player_t.Center = TransMat.MatrixMultple1x4x4x4(Player_t.Center);
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = TransMat.MatrixMultple1x4x4x4(Player_t.vertex[i]);
            }
            Player_t.Circoll = GetCircle(Player_t);
            CopyPack(Player_t, Current_Player_t);
            break;

            // D
        case 0x44:
            TranslateMatrix(TransMat, transVal, 0.0f, 0.0f);
            Player_t.Center = TransMat.MatrixMultple1x4x4x4(Player_t.Center);
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = TransMat.MatrixMultple1x4x4x4(Player_t.vertex[i]);
            }
            Player_t.Circoll = GetCircle(Player_t);
            CopyPack(Player_t, Current_Player_t);
            break;

            // G Rotate
        case 0x47:
        {
            Matrix4x4 RotateMat;
           
            
            // 1.원점으로 이동 행렬
            TranslateMatrix(OriginMat, -Player_t.Center.Get_X(), -Player_t.Center.Get_Y(), 0.0f);
            
            // 2.회전 행렬
            MatrixRotateZ(RotateMat, Degree_To_Radian(rotateVal));
            
            // 3.원래 자리로 이동 행렬
            TranslateMatrix(TransMat, Player_t.Center.Get_X(), Player_t.Center.Get_Y(), 0.0f);

            // 행렬 병합
            ResultMat = OriginMat * RotateMat;
            ResultMat = ResultMat * TransMat;
            
            // 병합된 행렬과 곱하기
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = ResultMat.MatrixMultple1x4x4x4(Player_t.vertex[i]);
            }
            Player_t.Circoll = GetCircle(Player_t);
            CopyPack(Player_t, Current_Player_t);
            break;
        }
            // F Scale
        case 0x46:
            Matrix4x4 ScalingMat;

            // 1.원점으로 이동 행렬
            TranslateMatrix(OriginMat, -Player_t.Center.Get_X(), -Player_t.Center.Get_Y(), 0.0f);

            // 2. 크기 증가 행렬
            MatrixScaling(ScalingMat, scaleVal, scaleVal, 1.0f); 

            // 3.원래 자리로 이동 행렬
            TranslateMatrix(TransMat, Player_t.Center.Get_X(), Player_t.Center.Get_Y(), 0.0f);
            
            // 행렬 병합
            ResultMat = OriginMat * ScalingMat;
            ResultMat = ResultMat * TransMat;

            // 병합된 행렬과 곱하기
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = ResultMat.MatrixMultple1x4x4x4(Player_t.vertex[i]);
            }
            Player_t.Circoll = GetCircle(Player_t);
            CopyPack(Player_t, Current_Player_t);
            break;
        }
        
        InvalidateRect(hWnd, NULL, FALSE);
        break;

        /* 화면 출력 */
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        backdc = CreateCompatibleDC(hdc);

        BackBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
        OldBackBitmap = (HBITMAP)SelectObject(backdc, BackBitmap);
        FillRect(backdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // 플레이어
        if (!ChangeColor)
        {
            //Ellipse(backdc, Player_t.Center.Get_X() - 5, Player_t.Center.Get_Y() - 5, Player_t.Center.Get_X() + 5, Player_t.Center.Get_Y() + 5); // 삼각형 중심점
            DrawPrimitive(backdc, Current_Player_t); // 삼각형 그리기
        }

        DrawCircle(backdc, Player_t.Circoll); // 원 그리는 함수(삼각함수 이용)

        MonsterManager(backdc, Monster, Current_Enemy_t);
        
        /* 현재 중심점에서 클릭좌표 까지의 직선의 방정식 */
        if (MouseX != NULL && MouseY != NULL) // 처음에 마우스 좌표값 없을때 예외처리
        {
            Equation(Player_t.Center.Get_X(), Player_t.Center.Get_Y(), MouseX, MouseY, &a, &b); // 기울기와 절편 구하기
            DrawLine(backdc, Line, a, b, Player_t.Center.Get_X(), MouseX); // 직선의 방정식 이용해서 그래프 그리기
            Ellipse(backdc, MouseX - 5, MouseY - 5, MouseX + 5, MouseY + 5); // 클릭한 곳
        }

        BitBlt(hdc, 0, 0, rt.right, rt.bottom, backdc, 0, 0, SRCCOPY);

        SelectObject(backdc, OldBackBitmap);
        DeleteDC(backdc);
        DeleteObject(BackBitmap);
        EndPaint(hWnd, &ps);
        break;
    
    case WM_DESTROY:
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        return 0;
    }
    return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

bool CircleCollision(CircleCollider p_player, CircleCollider p_monster)
{
    /* 플레이어 콜라이더 중심좌표, 몬스터 콜라이더 중심좌표, 플레이어 콜라이더 반지름 + 몬스터 콜라이더 반지름 */
    if (Alert(p_player.Center.Get_X(), p_player.Center.Get_Y(),
        p_monster.Center.Get_X(), p_monster.Center.Get_Y(), p_player.Radius + p_monster.Radius)) return true;
    else return false;
}

CircleCollider GetCircle(triangle p_t)
{
    CircleCollider retcoll;
    memset(&retcoll, 0, sizeof(CircleCollider));

    // 콜라이더 중점 좌표 계산
    retcoll.Center.Set_pos((p_t.vertex[0].Get_X() + p_t.vertex[1].Get_X() + p_t.vertex[2].Get_X()) / 3.0f
        , (p_t.vertex[0].Get_Y() + p_t.vertex[1].Get_Y() + p_t.vertex[2].Get_Y()) / 3.0f, 0.0f);

    float length[3]; // 중점 까지의 거리

    for (int i = 0; i < 3; i++)
    {
        length[i] = Watching(p_t.Center.Get_X(), p_t.Center.Get_Y(), p_t.vertex[i].Get_X(), p_t.vertex[i].Get_Y()); // 각 점에서 중점까지의 거리
    }

    float shortest = length[0];
    for (int i = 0; i < 3; i++)
    {
        if (length[i] <= shortest)
        {
            shortest = length[i]; 
        }
    }

    retcoll.Radius = shortest; // 가장 작은 값
    retcoll.coll = false; // 충돌 처리 변수

    return retcoll;
}

void MonsterManager(HDC p_backdc, Monster* p_monster, triangle* p_cur_t)
{
    EnemyCount = 5;

    for (int i = 0; i < EnemyCount; i++)
    {
        ChangeColor = true;
        //Ellipse(p_backdc, p_monster[i].Enemy_t.Center.Get_X() - 5, p_monster[i].Enemy_t.Center.Get_Y() - 5,
        //    p_monster[i].Enemy_t.Center.Get_X() + 5, p_monster[i].Enemy_t.Center.Get_Y() + 5); // 삼각형 중심점
        //for (int j = 0; j < WAY_POINT_COUNT; j++)
        //{
        //    // Way_Point
        //    Ellipse(p_backdc, p_monster[i].Way_Point[j].Get_X() - 5, p_monster[i].Way_Point[j].Get_Y() - 5,
        //        p_monster[i].Way_Point[j].Get_X() + 5, p_monster[i].Way_Point[j].Get_Y() + 5);
        //}
        DrawPrimitive(p_backdc, p_cur_t[i]); // 삼각형 그리기
        DrawCircle(p_backdc, p_monster[i].Enemy_t.Circoll); // 원 그리기(삼각함수 이용)
        
        ChangeColor = false;
    }
}

float Degree_To_Radian(float angle)
{
    /* 호도법(실수로 각을 표현하는 방법) */
    float arc; // 호의 길이
    float radian; // 라디안

    arc = (2 * PIE * RADIUS) * (angle / 360.0f); // 2πr * 중심각/360 = 호의 길이
    radian = arc / RADIUS; // 호의길이/반지름 = 라디안 

    return radian;
}

Matrix4x4 EnemyMoveDirection(Vec3D p_x1, Vec3D p_x2, float a)
{
    Matrix4x4 retMat;

    if (p_x1.Get_X() > p_x2.Get_X())
    {
        TranslateMatrix(retMat, -ENEMY_SPEED, -(a * ENEMY_SPEED), 0.0f);
    }
    if (p_x1.Get_X() < p_x2.Get_X())
    {
        TranslateMatrix(retMat, ENEMY_SPEED, a * ENEMY_SPEED, 0.0f);
    }

    // 기울기 예외처리
    if (p_x1.Get_X() == p_x2.Get_X() && p_x1.Get_Y() < p_x2.Get_Y())
    {
        TranslateMatrix(retMat, 0, ENEMY_SPEED, 0.0f);
    }
    if (p_x1.Get_X() == p_x2.Get_X() && p_x1.Get_Y() > p_x2.Get_Y())
    {
        TranslateMatrix(retMat, 0, -ENEMY_SPEED, 0.0f);
    }

    return retMat;
}

void MoveEnemy(Monster* p_mob, triangle* p_cur_t, Matrix4x4& p_transmat)
{
    p_mob->Enemy_t.Center = p_transmat.MatrixMultple1x4x4x4(p_mob->Enemy_t.Center); // 중심점 이동
    for (int k = 0; k < 3; k++)
    {
        p_mob->Enemy_t.vertex[k] = p_transmat.MatrixMultple1x4x4x4(p_mob->Enemy_t.vertex[k]); // 세 점 이동
    }
    p_mob->Enemy_t.Circoll = GetCircle(p_mob->Enemy_t);
    CopyPack(p_mob->Enemy_t, *p_cur_t);
}

/* 두 점사이의 거리를 구하는 함수 */
double Watching(int x1, int y1, int x2, int y2)
{
    return(sqrt(pow((x2 - x1),2) + pow((y2 - y1),2)));
}

BOOL Alert(int x, int y, int mx, int my, const int radius)
{
    // 두 점의 거리가 radius 보다 작으면 return true
    if (Watching(x, y, mx, my) < radius) return true; 
    else return false;
}

void MatrixScaling(Matrix4x4& p_mat, float scaleX, float scaleY, float scaleZ)
{
    p_mat = Matrix4x4
      (scaleX, 0.0f, 0.0f, 0.0f,
         0.0f, scaleY, 0.0f, 0.0f,
         0.0f, 0.0f, scaleZ, 0.0f,
         0.0f, 0.0f, 0.0f, 1.0f);
}
void MatrixRotateX(Matrix4x4& p_mat, float angle)
{
    p_mat = Matrix4x4
       (1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosf(angle), sinf(angle), 0.0f,
        0.0f, -sinf(angle), cosf(angle), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

void MatrixRotateY(Matrix4x4& p_mat, float angle)
{
    p_mat = Matrix4x4
       (cosf(angle), 0.0f, -sinf(angle), 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sinf(angle), 1.0f, cosf(angle), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

void MatrixRotateZ(Matrix4x4& p_mat, float angle)
{
    p_mat = Matrix4x4
        (cosf(angle), sinf(angle), 0.0f, 0.0f,
        -sinf(angle), cosf(angle), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}
void TranslateMatrix(Matrix4x4& p_mat, float p_dx, float p_dy, float p_dz)
{
    p_mat = Matrix4x4
       (1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        p_dx, p_dy, p_dz, 1.0f);
}

void CopyPack(triangle& t, triangle& Current_t)
{
    for (int i = 0; i < 3; i++)
    {
        Current_t.vertex[i] = t.vertex[i];
    }
    Current_t.Center = t.Center;
    Current_t.Circoll = t.Circoll;
}

void MoveTriangle(triangle& t, float p_x, float p_y) // p_x : 마우스 클릭 x좌표값 p_y : 마우스 클릭 y좌표값
{
    t.vertex[0].Set_pos(p_x, p_y - 50.0f, 1.0f);
    t.vertex[1].Set_pos(p_x - 50.0f, p_y + 50.0f, 1.0f);
    t.vertex[2].Set_pos(p_x + 50.0f, p_y + 50.0f, 1.0f);
}

void DrawPrimitive(HDC hdc, triangle& t)
{
    float a, b; // a = 기울기, b = 절편
    POINT Line[SIZE]; // 직선의 방정식에 사용할 변수
    
    Equation(t.vertex[0].Get_X(), t.vertex[0].Get_Y(), t.vertex[1].Get_X(), t.vertex[1].Get_Y(), &a, &b); // 기울기와 절편 구하기
    DrawLine(hdc, Line, a, b, t.vertex[0].Get_X(), t.vertex[1].Get_X()); // 직선의 방정식 이용해서 그래프 그리기

    Equation(t.vertex[1].Get_X(), t.vertex[1].Get_Y(), t.vertex[2].Get_X(), t.vertex[2].Get_Y(), &a, &b); // 기울기와 절편 구하기
    DrawLine(hdc, Line, a, b, t.vertex[1].Get_X(), t.vertex[2].Get_X()); // 직선의 방정식 이용해서 그래프 그리기

    Equation(t.vertex[2].Get_X(), t.vertex[2].Get_Y(), t.vertex[0].Get_X(), t.vertex[0].Get_Y(), &a, &b); // 기울기와 절편 구하기
    DrawLine(hdc, Line, a, b, t.vertex[2].Get_X(), t.vertex[0].Get_X()); // 직선의 방정식 이용해서 그래프 그리기
} 

/* 두 점을 알때 기울기, 절편 구하기 */
void Equation(float x1, float y1, float x2, float y2, float* a, float* b)
{
   
    if (x2 != x1) // 분모가 0이 되지 않도록 처리
        *a = (y2 - y1) / (x2 - x1); // 기울기(a) = (y2-y1) / (x2-x1)

    *b = y1 - *a * x1; // 절편(b) = y - ax
    
}

/* 직선의 방정식 , 그래프 그리기 */
void DrawLine(HDC backdc, POINT* Line, float a, float b, float p_x1, float p_x2)
{
    // a 는 기울기 b 는 절편
    // 빗변
    if (p_x2 > p_x1)
    {
        for (int i = p_x1; i < p_x2; i++) 
        {
            Line[i].x = i;
            Line[i].y = a * Line[i].x + b; // y = ax + b

            SetPixel(backdc, Line[i].x, Line[i].y, RGB(0, 0, 0));
            if (ChangeColor) // 몬스터
            {
                SetPixel(backdc, Line[i].x, Line[i].y, RGB(255, 0, 0));
                
            }
        }
    }
    // 밑변
    if (p_x1 > p_x2)
    {
        for (int i = p_x2; i < p_x1; i++) 
        {
            Line[i].x = i;
            Line[i].y = a * Line[i].x + b; // y = ax + b

            SetPixel(backdc, Line[i].x, Line[i].y, RGB(0, 0, 0));
            if (ChangeColor) // 몬스터
            {
                SetPixel(backdc, Line[i].x, Line[i].y, RGB(255, 0, 0)); 
            }
        }
    }
}

void DrawCircle(HDC backdc, CircleCollider p_coll)
{
    POINT Line[SIZE];

    /* 삼각 함수를 이용한 원 그리기(원의 반지름이 빗변) */
    for (int angle = 0; angle <= 360; angle++)
    {
        Line[angle].x = (cos(Degree_To_Radian(angle)) * p_coll.Radius) + p_coll.Center.Get_X(); // x = cos(radian) * radius
        Line[angle].y = (sin(Degree_To_Radian(angle)) * p_coll.Radius) + p_coll.Center.Get_Y(); // y = sin(radian) * radius
        
        SetPixel(backdc, Line[angle].x, Line[angle].y, RGB(0, 0, 0)); 
        
        if (ChangeColor) // 몬스터 색
        {
            if (p_coll.coll) // 플레이어와 충돌(파란색)
            {
                SetPixel(backdc, Line[angle].x, Line[angle].y, RGB(0, 0, 255));
            }
            else // (빨간색)
            {
                SetPixel(backdc, Line[angle].x, Line[angle].y, RGB(255, 0, 0));
            }
        }
    }
}