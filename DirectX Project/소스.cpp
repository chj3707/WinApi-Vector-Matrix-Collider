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
#define PIE 3.1415 // ������

#define RADIUS 25 // �÷��̾� ������

#define ENEMY_RADIUS 10 // ���� ������
#define ENEMY_SPEED 1.0 // ���� �ӵ�
#define ENEMY_MAXCOUNT 100 // ���� ��ü �ִ� ����
#define WAY_POINT_COUNT 5 // WayPoint ����
#define ALERTVALUE 100 // ���Ͱ� �÷��̾ �����ϴ� �Ÿ�

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

/* �Լ� ���� */
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
    static POINT Line[SIZE]; // ������ �����Ŀ� ����� ����
    
    static triangle Player_t; // �÷��̾�
    static triangle Current_Player_t;

    static Matrix4x4 EnemyTransMat; // ���� �̵� ���
    static Monster Monster[ENEMY_MAXCOUNT]; // ����
    static triangle Current_Enemy_t[ENEMY_MAXCOUNT];

    static float MouseX, MouseY; // ���콺 Ŭ�� ��ǥ
    static bool Clickflag; // ���콺 Ŭ�� �÷���

    static Matrix4x4 TransMat; // �̵� ���
    static Matrix4x4 OriginMat; // �������� �̵��ϴ� ���
    static Matrix4x4 ResultMat; // ��� ���� ���
    static float transVal, rotateVal, scaleVal;

    static float a, b; // ����, ����

    //static TCHAR text1[256], text2[256], text3[256], text4[256]; // ��ǥ�� Ȯ�ο� 
   
    switch (iMessage) {

        /* ���� �ʱⰪ ���� */
    case WM_CREATE:
        GetClientRect(hWnd, &rt); // Ŭ���̾�Ʈ ũ�� (1424,720)
        srand(time(NULL));

        transVal = 30.0f; // �̵� ��
        rotateVal = 10.0f; // ȸ�� ��
        scaleVal = 1.1f; // ũ�� ���� ��

        /* �ﰢ�� �߽� ��ǥ �ʱⰪ ���� */
        Player_t.Center = Vec3D(rt.right / 2, rt.bottom / 2, 1.0f);
        
        /* �ﰢ�� �� ���� ��ǥ �ʱⰪ ���� */
        Player_t.vertex[0] = Vec3D(Player_t.Center.Get_X(), Player_t.Center.Get_Y() - RADIUS, 1.0f);
        Player_t.vertex[1] = Vec3D(Player_t.Center.Get_X() - RADIUS, Player_t.Center.Get_Y() + RADIUS, 1.0f);
        Player_t.vertex[2] = Vec3D(Player_t.Center.Get_X() + RADIUS, Player_t.Center.Get_Y() + RADIUS, 1.0f);
        
        Player_t.Circoll = GetCircle(Player_t);
        CopyPack(Player_t, Current_Player_t); // ������ ��ǥ ����

        /* WayPoint ���� */
        for (int i = 0; i < ENEMY_MAXCOUNT; i++)
        {
            for (int j = 0; j < WAY_POINT_COUNT; j++)
            {
                // Ŭ���̾�Ʈ ũ�� �ȿ� ���� ��ġ�� ����Ʈ ����
                Monster[i].Way_Point[j] = Vec3D(rand() % (rt.right - (RADIUS * 2)) + (rt.left + RADIUS), rand() % (rt.bottom - (RADIUS * 2)) + (rt.top + RADIUS), 1.0f);
            }
        }

        for (int i = 0; i < ENEMY_MAXCOUNT; i++)
        {
            /* ����(�ﰢ��) ���� ��ġ . ù��° ����Ʈ�� ���� */
            Monster[i].Enemy_t.Center = Vec3D(Monster[i].Way_Point[0].Get_X(), Monster[i].Way_Point[0].Get_Y(), 1.0f);
            
            /* ����(�ﰢ��) ũ�� ���� */
            Monster[i].Enemy_t.vertex[0] = Vec3D(Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y() - ENEMY_RADIUS, 1.0f);
            Monster[i].Enemy_t.vertex[1] = Vec3D(Monster[i].Enemy_t.Center.Get_X() - ENEMY_RADIUS, Monster[i].Enemy_t.Center.Get_Y() + ENEMY_RADIUS, 1.0f);
            Monster[i].Enemy_t.vertex[2] = Vec3D(Monster[i].Enemy_t.Center.Get_X() + ENEMY_RADIUS, Monster[i].Enemy_t.Center.Get_Y() + ENEMY_RADIUS, 1.0f);

            Monster[i].Point_Count = 0;
            Monster[i].Enemy_t.Circoll = GetCircle(Monster[i].Enemy_t);

            CopyPack(Monster[i].Enemy_t, Current_Enemy_t[i]);
        }

        Clickflag = false; // ���콺 Ŭ�� �÷���

        SetTimer(hWnd, 1, 10, NULL); 
        break;

        /* ���콺 Ŭ�� */
    case WM_LBUTTONDOWN:
        /*MouseX = LOWORD(lParam);
        MouseY = HIWORD(lParam);

        Clickflag = true;*/
        
        break;

        /* Ÿ�̸� */
    case WM_TIMER:
        switch (wParam)
        {
        case 1:
            
            for (int i = 0; i < EnemyCount; i++)
            {
                /* �÷��̾�� ���� ����(Follow) */
                if (Alert(Player_t.Center.Get_X(), Player_t.Center.Get_Y(),
                    Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(), ALERTVALUE)) // �÷��̾� �߽���ǥ, ���� �߽���ǥ
                {
                    /* �÷��̾��� �߽���ǥ�� ���� �߽���ǥ�� ���� */
                    Equation(Player_t.Center.Get_X(), Player_t.Center.Get_Y(),
                        Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(), &a, &b);

                    EnemyTransMat = EnemyMoveDirection(Monster[i].Enemy_t.Center, Player_t.Center, a);

                    MoveEnemy(&Monster[i], &Current_Enemy_t[i], EnemyTransMat);

                    /* �ݶ��̴� �浹 üũ */
                    if (CircleCollision(Player_t.Circoll, Monster[i].Enemy_t.Circoll))
                    {
                        Monster[i].Enemy_t.Circoll.coll = true;
                    }
                    else
                    {
                        Monster[i].Enemy_t.Circoll.coll = false;
                    }
                }

                /* ���Ͱ� WayPoint�� ����(���� WayPoint�� �̵�) */
                else if (Alert(Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(),
                    Monster[i].Way_Point[Monster[i].Point_Count].Get_X(), Monster[i].Way_Point[Monster[i].Point_Count].Get_Y(), ENEMY_RADIUS)) // ���� �߽���ǥ, WayPoint ��ǥ
                {
                    
                    ++Monster[i].Point_Count;

                    /* ������ ����Ʈ�� �����ϸ� ī��Ʈ �ʱ�ȭ(������ ����Ʈ -> ù��° ����Ʈ�� �̵�) */
                    if (Monster[i].Point_Count == WAY_POINT_COUNT)
                    {
                        Monster[i].Point_Count = 0;
                    }
                }

                else
                {
                    /* ������ �߽���ǥ�� WayPoint��ǥ�� ���� */
                    Equation(Monster[i].Enemy_t.Center.Get_X(), Monster[i].Enemy_t.Center.Get_Y(),
                        Monster[i].Way_Point[Monster[i].Point_Count].Get_X(), Monster[i].Way_Point[Monster[i].Point_Count].Get_Y(), &a, &b);

                    EnemyTransMat = EnemyMoveDirection(Monster[i].Enemy_t.Center, Monster[i].Way_Point[Monster[i].Point_Count], a);

                    MoveEnemy(&Monster[i], &Current_Enemy_t[i], EnemyTransMat);

                    /* �ݶ��̴� �浹 üũ(Follow ���� �� Ȯ�ο�) */
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

        /* Ű �Է� */
    case WM_KEYDOWN:
        switch (wParam)
        {
            //https://shaeod.tistory.com/388 Ű �ڵ�

            // W
        case 0x57:
            TranslateMatrix(TransMat, 0.0f, -transVal, 0.0f); // �̵���� �� ����
            Player_t.Center = TransMat.MatrixMultple1x4x4x4(Player_t.Center); // �߽��� �̵�
            for (int i = 0; i < 3; i++)
            {
                Player_t.vertex[i] = TransMat.MatrixMultple1x4x4x4(Player_t.vertex[i]); // �� �� �̵�
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
           
            
            // 1.�������� �̵� ���
            TranslateMatrix(OriginMat, -Player_t.Center.Get_X(), -Player_t.Center.Get_Y(), 0.0f);
            
            // 2.ȸ�� ���
            MatrixRotateZ(RotateMat, Degree_To_Radian(rotateVal));
            
            // 3.���� �ڸ��� �̵� ���
            TranslateMatrix(TransMat, Player_t.Center.Get_X(), Player_t.Center.Get_Y(), 0.0f);

            // ��� ����
            ResultMat = OriginMat * RotateMat;
            ResultMat = ResultMat * TransMat;
            
            // ���յ� ��İ� ���ϱ�
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

            // 1.�������� �̵� ���
            TranslateMatrix(OriginMat, -Player_t.Center.Get_X(), -Player_t.Center.Get_Y(), 0.0f);

            // 2. ũ�� ���� ���
            MatrixScaling(ScalingMat, scaleVal, scaleVal, 1.0f); 

            // 3.���� �ڸ��� �̵� ���
            TranslateMatrix(TransMat, Player_t.Center.Get_X(), Player_t.Center.Get_Y(), 0.0f);
            
            // ��� ����
            ResultMat = OriginMat * ScalingMat;
            ResultMat = ResultMat * TransMat;

            // ���յ� ��İ� ���ϱ�
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

        /* ȭ�� ��� */
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        backdc = CreateCompatibleDC(hdc);

        BackBitmap = CreateCompatibleBitmap(hdc, rt.right, rt.bottom);
        OldBackBitmap = (HBITMAP)SelectObject(backdc, BackBitmap);
        FillRect(backdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));

        // �÷��̾�
        if (!ChangeColor)
        {
            //Ellipse(backdc, Player_t.Center.Get_X() - 5, Player_t.Center.Get_Y() - 5, Player_t.Center.Get_X() + 5, Player_t.Center.Get_Y() + 5); // �ﰢ�� �߽���
            DrawPrimitive(backdc, Current_Player_t); // �ﰢ�� �׸���
        }

        DrawCircle(backdc, Player_t.Circoll); // �� �׸��� �Լ�(�ﰢ�Լ� �̿�)

        MonsterManager(backdc, Monster, Current_Enemy_t);
        
        /* ���� �߽������� Ŭ����ǥ ������ ������ ������ */
        if (MouseX != NULL && MouseY != NULL) // ó���� ���콺 ��ǥ�� ������ ����ó��
        {
            Equation(Player_t.Center.Get_X(), Player_t.Center.Get_Y(), MouseX, MouseY, &a, &b); // ����� ���� ���ϱ�
            DrawLine(backdc, Line, a, b, Player_t.Center.Get_X(), MouseX); // ������ ������ �̿��ؼ� �׷��� �׸���
            Ellipse(backdc, MouseX - 5, MouseY - 5, MouseX + 5, MouseY + 5); // Ŭ���� ��
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
    /* �÷��̾� �ݶ��̴� �߽���ǥ, ���� �ݶ��̴� �߽���ǥ, �÷��̾� �ݶ��̴� ������ + ���� �ݶ��̴� ������ */
    if (Alert(p_player.Center.Get_X(), p_player.Center.Get_Y(),
        p_monster.Center.Get_X(), p_monster.Center.Get_Y(), p_player.Radius + p_monster.Radius)) return true;
    else return false;
}

CircleCollider GetCircle(triangle p_t)
{
    CircleCollider retcoll;
    memset(&retcoll, 0, sizeof(CircleCollider));

    // �ݶ��̴� ���� ��ǥ ���
    retcoll.Center.Set_pos((p_t.vertex[0].Get_X() + p_t.vertex[1].Get_X() + p_t.vertex[2].Get_X()) / 3.0f
        , (p_t.vertex[0].Get_Y() + p_t.vertex[1].Get_Y() + p_t.vertex[2].Get_Y()) / 3.0f, 0.0f);

    float length[3]; // ���� ������ �Ÿ�

    for (int i = 0; i < 3; i++)
    {
        length[i] = Watching(p_t.Center.Get_X(), p_t.Center.Get_Y(), p_t.vertex[i].Get_X(), p_t.vertex[i].Get_Y()); // �� ������ ���������� �Ÿ�
    }

    float shortest = length[0];
    for (int i = 0; i < 3; i++)
    {
        if (length[i] <= shortest)
        {
            shortest = length[i]; 
        }
    }

    retcoll.Radius = shortest; // ���� ���� ��
    retcoll.coll = false; // �浹 ó�� ����

    return retcoll;
}

void MonsterManager(HDC p_backdc, Monster* p_monster, triangle* p_cur_t)
{
    EnemyCount = 5;

    for (int i = 0; i < EnemyCount; i++)
    {
        ChangeColor = true;
        //Ellipse(p_backdc, p_monster[i].Enemy_t.Center.Get_X() - 5, p_monster[i].Enemy_t.Center.Get_Y() - 5,
        //    p_monster[i].Enemy_t.Center.Get_X() + 5, p_monster[i].Enemy_t.Center.Get_Y() + 5); // �ﰢ�� �߽���
        //for (int j = 0; j < WAY_POINT_COUNT; j++)
        //{
        //    // Way_Point
        //    Ellipse(p_backdc, p_monster[i].Way_Point[j].Get_X() - 5, p_monster[i].Way_Point[j].Get_Y() - 5,
        //        p_monster[i].Way_Point[j].Get_X() + 5, p_monster[i].Way_Point[j].Get_Y() + 5);
        //}
        DrawPrimitive(p_backdc, p_cur_t[i]); // �ﰢ�� �׸���
        DrawCircle(p_backdc, p_monster[i].Enemy_t.Circoll); // �� �׸���(�ﰢ�Լ� �̿�)
        
        ChangeColor = false;
    }
}

float Degree_To_Radian(float angle)
{
    /* ȣ����(�Ǽ��� ���� ǥ���ϴ� ���) */
    float arc; // ȣ�� ����
    float radian; // ����

    arc = (2 * PIE * RADIUS) * (angle / 360.0f); // 2��r * �߽ɰ�/360 = ȣ�� ����
    radian = arc / RADIUS; // ȣ�Ǳ���/������ = ���� 

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

    // ���� ����ó��
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
    p_mob->Enemy_t.Center = p_transmat.MatrixMultple1x4x4x4(p_mob->Enemy_t.Center); // �߽��� �̵�
    for (int k = 0; k < 3; k++)
    {
        p_mob->Enemy_t.vertex[k] = p_transmat.MatrixMultple1x4x4x4(p_mob->Enemy_t.vertex[k]); // �� �� �̵�
    }
    p_mob->Enemy_t.Circoll = GetCircle(p_mob->Enemy_t);
    CopyPack(p_mob->Enemy_t, *p_cur_t);
}

/* �� �������� �Ÿ��� ���ϴ� �Լ� */
double Watching(int x1, int y1, int x2, int y2)
{
    return(sqrt(pow((x2 - x1),2) + pow((y2 - y1),2)));
}

BOOL Alert(int x, int y, int mx, int my, const int radius)
{
    // �� ���� �Ÿ��� radius ���� ������ return true
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

void MoveTriangle(triangle& t, float p_x, float p_y) // p_x : ���콺 Ŭ�� x��ǥ�� p_y : ���콺 Ŭ�� y��ǥ��
{
    t.vertex[0].Set_pos(p_x, p_y - 50.0f, 1.0f);
    t.vertex[1].Set_pos(p_x - 50.0f, p_y + 50.0f, 1.0f);
    t.vertex[2].Set_pos(p_x + 50.0f, p_y + 50.0f, 1.0f);
}

void DrawPrimitive(HDC hdc, triangle& t)
{
    float a, b; // a = ����, b = ����
    POINT Line[SIZE]; // ������ �����Ŀ� ����� ����
    
    Equation(t.vertex[0].Get_X(), t.vertex[0].Get_Y(), t.vertex[1].Get_X(), t.vertex[1].Get_Y(), &a, &b); // ����� ���� ���ϱ�
    DrawLine(hdc, Line, a, b, t.vertex[0].Get_X(), t.vertex[1].Get_X()); // ������ ������ �̿��ؼ� �׷��� �׸���

    Equation(t.vertex[1].Get_X(), t.vertex[1].Get_Y(), t.vertex[2].Get_X(), t.vertex[2].Get_Y(), &a, &b); // ����� ���� ���ϱ�
    DrawLine(hdc, Line, a, b, t.vertex[1].Get_X(), t.vertex[2].Get_X()); // ������ ������ �̿��ؼ� �׷��� �׸���

    Equation(t.vertex[2].Get_X(), t.vertex[2].Get_Y(), t.vertex[0].Get_X(), t.vertex[0].Get_Y(), &a, &b); // ����� ���� ���ϱ�
    DrawLine(hdc, Line, a, b, t.vertex[2].Get_X(), t.vertex[0].Get_X()); // ������ ������ �̿��ؼ� �׷��� �׸���
} 

/* �� ���� �˶� ����, ���� ���ϱ� */
void Equation(float x1, float y1, float x2, float y2, float* a, float* b)
{
   
    if (x2 != x1) // �и� 0�� ���� �ʵ��� ó��
        *a = (y2 - y1) / (x2 - x1); // ����(a) = (y2-y1) / (x2-x1)

    *b = y1 - *a * x1; // ����(b) = y - ax
    
}

/* ������ ������ , �׷��� �׸��� */
void DrawLine(HDC backdc, POINT* Line, float a, float b, float p_x1, float p_x2)
{
    // a �� ���� b �� ����
    // ����
    if (p_x2 > p_x1)
    {
        for (int i = p_x1; i < p_x2; i++) 
        {
            Line[i].x = i;
            Line[i].y = a * Line[i].x + b; // y = ax + b

            SetPixel(backdc, Line[i].x, Line[i].y, RGB(0, 0, 0));
            if (ChangeColor) // ����
            {
                SetPixel(backdc, Line[i].x, Line[i].y, RGB(255, 0, 0));
                
            }
        }
    }
    // �غ�
    if (p_x1 > p_x2)
    {
        for (int i = p_x2; i < p_x1; i++) 
        {
            Line[i].x = i;
            Line[i].y = a * Line[i].x + b; // y = ax + b

            SetPixel(backdc, Line[i].x, Line[i].y, RGB(0, 0, 0));
            if (ChangeColor) // ����
            {
                SetPixel(backdc, Line[i].x, Line[i].y, RGB(255, 0, 0)); 
            }
        }
    }
}

void DrawCircle(HDC backdc, CircleCollider p_coll)
{
    POINT Line[SIZE];

    /* �ﰢ �Լ��� �̿��� �� �׸���(���� �������� ����) */
    for (int angle = 0; angle <= 360; angle++)
    {
        Line[angle].x = (cos(Degree_To_Radian(angle)) * p_coll.Radius) + p_coll.Center.Get_X(); // x = cos(radian) * radius
        Line[angle].y = (sin(Degree_To_Radian(angle)) * p_coll.Radius) + p_coll.Center.Get_Y(); // y = sin(radian) * radius
        
        SetPixel(backdc, Line[angle].x, Line[angle].y, RGB(0, 0, 0)); 
        
        if (ChangeColor) // ���� ��
        {
            if (p_coll.coll) // �÷��̾�� �浹(�Ķ���)
            {
                SetPixel(backdc, Line[angle].x, Line[angle].y, RGB(0, 0, 255));
            }
            else // (������)
            {
                SetPixel(backdc, Line[angle].x, Line[angle].y, RGB(255, 0, 0));
            }
        }
    }
}