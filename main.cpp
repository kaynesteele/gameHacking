#include "main.h"
#include <iostream>
#include "aim.h"
#include "offsets.h"
#include <algorithm>
#include <math.h>

void test();
BOOL WINAPI DllMain(HINSTANCE Instance, DWORD Reason, LPVOID Reserved)
{
	if (Reason == DLL_PROCESS_ATTACH)
	{
		AllocConsole();

		HWND hwnd = FindWindow(NULL, "Counter-Strike: Global Offensive");

		HMENU hMenu = GetSystemMenu(hwnd, FALSE);
		if (hMenu) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);

		SetConsoleTitle("");
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)test, NULL, NULL, NULL);
	}
	return true;
}

float Get3dDistance(float myCoordsX, float myCoordsZ, float myCoordsY,
                    float enX, float enZ, float enY)
{

    return sqrt(pow(double(enX - myCoordsX), 2.0) +
            pow(double(enY - myCoordsY), 2.0) +
            pow(double(enZ - myCoordsZ), 2.0));

}

float * CalcAngle(float *src, float *dst,float *angles)
{
	double delta[3] = {(src[0]-dst[0]), (src[1]-dst[1]), (src[2]-dst[2])};
	double hyp = sqrt(delta[0]*delta[0] + delta[1]*delta[1]);
	angles[0] = (float) (atanf(delta[2]/hyp) * 57.295779513682f);
	angles[1] = (float) (atanf(delta[1]/delta[0]) * 57.295779513682f);
	angles[2] = 0;

	if(delta[0] >= 0.0)
	{
		angles[1] += 180.0f;
	}
	return angles;

}

struct TargetList_t
{
	float Distance;
	float vecVel[3];
	float AimbotAngle[3];

	TargetList_t()
	{
	}

	TargetList_t(float aimbotAngle[], float myCoords[], float enemyCoords[])
	{
		Distance = Get3dDistance(myCoords[0], myCoords[1], myCoords[2],
				aimbotAngle[0], aimbotAngle[1], aimbotAngle[2]);

				//Distance = DifferenceOfAngles(myCoords, aimbotAngle);
		AimbotAngle[0] = aimbotAngle[0];
		AimbotAngle[1] = aimbotAngle[1];
		AimbotAngle[2] = aimbotAngle[2];

	}
};

struct CompareTargetEnArray
{
    bool operator () (TargetList_t & lhs, TargetList_t & rhs)
	{
		return lhs.Distance < rhs.Distance;
	}
};
float * addArrs(float* arr1, float *arr2, int x)
{
    float * finalResult = new float[3];

    for(int i = 0; i < x; i++)
    {
        finalResult[i] = (arr1[i] + arr2[i]);
    }
    finalResult[3] = 100;

    return finalResult;
}

float * subArrs(float* arr1, float *arr2, int x)
{
    float * finalResult = new float[3];

    for(int i = 0; i < x; i++)
    {
        finalResult[i] = (arr1[i] - arr2[i] *2.0f);
    }

    return finalResult;
}

float flAngleNormalize(float angle)
{
	while (angle < -180)    angle += 360;
	while (angle > 180)    angle -= 360;
	return angle;
}
float * ClampAngle(float * angles){
	angles[1] = flAngleNormalize(angles[1]);
	angles[0] = flAngleNormalize(angles[0]);
	if (angles[0] > 89.0f)
		angles[0] = 89.0f;
	if (angles[0] < -89.0f)
		angles[0] = -89.0f;

	angles[2] = 0;
	return angles;

}

void hop(DWORD *localPlayer, DWORD gameModule)
{
    if(*localPlayer!=NULL)
    {
        int *flags = (int*)(*localPlayer + m_fFlags);
        if(*flags == 257)
            *(DWORD*)(gameModule+dwForceJump)=6;
    }
}

void thirdPerson(DWORD gameModule, DWORD engineModule, DWORD *localPlayer)
{
    DWORD *clientState = (DWORD*)(engineModule + dwClientState);
    float *viewAngles = new float[3];
    viewAngles=(float*)(*clientState + dwClientState_ViewAngles);

    if(*localPlayer!=NULL)
    {
        *(int*)(*localPlayer+m_iObserverMode)=1;
        *(float*)(*localPlayer + m_vecViewOffset)=viewAngles[0];
        *(float*)(*localPlayer + m_vecViewOffset+0x4)=viewAngles[1];
        *(float*)(*localPlayer + m_vecViewOffset+0x8)=viewAngles[2]+180;

    }
}
void fovChanger(int fov)
{
    DWORD gameModule;
    gameModule = (DWORD)GetModuleHandle("client_panorama.dll");
    DWORD *localPlayer = (DWORD*)(gameModule + dwLocalPlayer);
    if(*localPlayer!=NULL)
        *(int*)(*localPlayer+m_iFOV)=fov;
}

void silentAim(DWORD engineModule, float *viewAngles, float *aimAngles)
{
    DWORD *clientState = (DWORD*)(engineModule + dwClientState);
   // ClampAngle(viewAngles);
    *(float*)(*clientState+dwClientState_ViewAngles)=aimAngles[0];
    *(float*)(*clientState+dwClientState_ViewAngles+0x4)=aimAngles[1];
    *(float*)(*clientState+dwClientState_ViewAngles+0x8)=aimAngles[2];
}

void glow(DWORD gameModule, DWORD *localPlayer)
{
    DWORD *glowObject = (DWORD*)(gameModule+dwGlowObjectManager);
    DWORD baseEntity;

    if(*localPlayer!=NULL)
    {

        int *myTeam = (int*)(*localPlayer + m_iTeamNum);

         for(int i = 0; i < 32; i++)
        {
            baseEntity=*(DWORD*)(gameModule + dwEntityList + (i * 0x10));
            if(baseEntity!=NULL)
            {
                int *Team = (int*)(baseEntity + m_iTeamNum);
                int glowIndex = *(int*)(baseEntity + m_iGlowIndex);
                if(*Team == *myTeam)
                {
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x4)=0;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x8)=0.0;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0xC)=2.0;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x10)=1.2;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x28)=2;


                }
                else
                {
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x4)=2;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x8)=0;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0xC)=1;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x10)=2;
                    *(float*)(*glowObject + glowIndex * 0x38 + 0x28)=1;

                }
                *(bool*)(*glowObject + glowIndex * 0x38 + 0x24) = true;
                *(bool*)(*glowObject + glowIndex * 0x38 + 0x25) = false;
            }
        }
    }



}
void bot(DWORD gameModule, DWORD engineModule, DWORD *localPlayer, int fov, bool walls)
{
    system("cls");
    TargetList_t * TargetList = new TargetList_t[32];
    DWORD baseEntity, boneMatrix;
    DWORD *clientState = (DWORD*)(engineModule + dwClientState);
    int *myTeam = (int*)(*localPlayer + m_iTeamNum);
    int place=fov;
    float *myPos = new float[3];
    float *myPunch = new float[3];
    float *aimAngle = new float[3];
    float *myViewAngle = new float[3];
    float *enemyBonePos = new float[3];
    float *enemyPos = new float[3];
    float *myEyePos = new float[3];
    float *finalAim = new float[3];
    float *vecVel = new float[3];
    float *vecVelArr = new float[3];
    float *lastAngle = new float[3];

    myPos = (float*)(*localPlayer+m_vecOrigin);
    myEyePos = (float*)(*localPlayer+m_vecViewOffset);
    myPunch = (float*)(*localPlayer+m_aimPunchAngle);

    float * test = new float[3];
    for(int i =0; i < 3; i++)
    {
        test[i] = myPos[i] + myEyePos[i];
    }
    myViewAngle=(float*)(*clientState + dwClientState_ViewAngles);
   // std::cout << myViewAngle[0] << " " << myViewAngle[1] << std::endl;

    //std::cout << myPos[0] << " " << myPos[1] << " " << myPos[2] << std::endl;
    int x = 0;

    for(int i = 1; i < 32; i++)
    {
        baseEntity=*(DWORD*)(gameModule + dwEntityList + (i * 0x10));

        if(baseEntity!=NULL)
        {
            enemyPos = (float*)(baseEntity+m_vecOrigin);
            boneMatrix = *(DWORD*)(baseEntity + m_dwBoneMatrix);

            int *Team = (int*)(baseEntity + m_iTeamNum);
            int *Hp = (int*)(baseEntity+m_iHealth);
            bool *dormant = (bool*)(baseEntity+m_bDormant);
            int *lifestate = (int*)(baseEntity+m_lifeState);
            int *spotted = (int*)(baseEntity+m_bSpotted);


            //std::cout << *lifestate << std::endl;
            enemyBonePos[0] = *(float*)(boneMatrix + 0x30 * 8 + 0xC);
            enemyBonePos[1] = *(float*)(boneMatrix + 0x30 * 8 + 0x1C);
            enemyBonePos[2] = *(float*)(boneMatrix + 0x30 * 8 + 0x2C);
            vecVel = (float*)(baseEntity+m_vecVelocity);

            if(!walls)
            {
                if(*Hp > 0 && *lifestate==0 && *spotted && *Team != *myTeam)
                {
                    vecVelArr = addArrs(enemyBonePos,vecVel,3);
                    CalcAngle(test, enemyBonePos, aimAngle);
                    ClampAngle(aimAngle);
                    //std::cout<<aimAngle[0]<<" " << aimAngle[1]<<std::endl;
                    TargetList[x] = TargetList_t((aimAngle), myViewAngle, enemyPos);
                    x++;
                }
            }
            else if(*Hp > 0 && *lifestate==0 && *Team != *myTeam)
            {
                vecVelArr = addArrs(enemyBonePos,vecVel,3);
                CalcAngle(test, enemyBonePos, aimAngle);
                ClampAngle(aimAngle);
                TargetList[x] = TargetList_t((aimAngle), myViewAngle, enemyPos);
                x++;
            }
           // std::cout << i << ": " << *Team << std::endl;
        }
    }
    if(x > 0)
    {
        std::sort(TargetList, TargetList+x, CompareTargetEnArray());

        if(GetAsyncKeyState(VK_LBUTTON))
        {
            fov++;
        }
        fov = place;

        if(TargetList[0].Distance < fov)
        {
            std::cout<<"AIMING" << std::endl;
            finalAim = subArrs(TargetList[0].AimbotAngle,myPunch,3);
            silentAim(engineModule, myViewAngle, finalAim);


        }
    }
    x = 0;
    delete[] aimAngle;
    delete[] TargetList;
}


void test()
{
    bool bunnyHop = false;
    bool bLag= false;
    bool bFov = false;
    bool aim = false;
    bool bGlow = false;
    bool bThird = false;
    bool bWalls = false;
    bool bAntiAim = false;
    int fov1=100;
    int fov=2;
    bool bSwitch = false;

    while(true)
    {
    DWORD gameModule = (DWORD)(GetModuleHandle("client_panorama.dll"));
    DWORD engineModule = (DWORD)GetModuleHandle("engine.dll");
    DWORD *localPlayer = (DWORD*)(gameModule + dwLocalPlayer);

        if(GetAsyncKeyState(VK_F1))
        {
            bunnyHop = !bunnyHop;
            if(bunnyHop)
                std::cout << "BunnyHop On" << std::endl;
            else
                std::cout << "BunnyHop Off" << std::endl;
            Sleep(100);
        }
        if(GetAsyncKeyState(VK_UP))
        {
            fov++;
            std::cout << fov << std::endl;
            Sleep(50);
        }
        if(GetAsyncKeyState(VK_DOWN))
        {
            if(fov>1)
            {
                fov--;
                std::cout << fov << std::endl;
                Sleep(50);
            }


            else
            {
                fov=1;
                std::cout << fov << std::endl;
                Sleep(50);
            }

        }
        if(GetAsyncKeyState(VK_LEFT))
        {
            if(fov1>1)
            {
                fov1--;
                std::cout << fov1 << std::endl;
                Sleep(50);
            }


            else
            {
                fov1=1;
                std::cout << fov1 << std::endl;
                Sleep(50);
            }

        }
        if(GetAsyncKeyState(VK_RIGHT))
        {

                fov1++;
                std::cout << fov1 << std::endl;
                Sleep(50);
        }



        if(GetAsyncKeyState(VK_F2))
        {
            aim = !aim;
            if(aim)
                std::cout << "Aim On" << std::endl;
            else
                std::cout << "Aim Off" << std::endl;
            Sleep(100);
        }
        if(GetAsyncKeyState(VK_F3))
        {
            bGlow = !bGlow;
            if(bGlow)
                std::cout << "Glow On" << std::endl;
            else
                std::cout << "Glow Off" << std::endl;
            Sleep(100);
        }
         if(GetAsyncKeyState(VK_F4))
        {
            bWalls = !bWalls;
            if(bWalls)
                std::cout << "Aim Through Walls On" << std::endl;
            else
                std::cout << "Aim Through Walls Off" << std::endl;
            Sleep(100);
        }

        if(GetAsyncKeyState(VK_F5))
        {
            bFov = !bFov;
            if(bFov)
                std::cout << "Fov: " << fov1 << std::endl;
            else
                fov1=90;
                if(*localPlayer!=NULL)
                    *(int*)(*localPlayer+m_iFOV)=fov1;
                std::cout << "Fov: " << fov1 << std::endl;

            Sleep(100);
        }


        if(bunnyHop && GetAsyncKeyState(VK_SPACE))
            hop(localPlayer, gameModule);

        if(aim && GetAsyncKeyState(VK_MENU))
            bot(gameModule,engineModule,localPlayer,fov, bWalls);

        if(bGlow)
            glow(gameModule, localPlayer);

        if(bFov)
            fovChanger(fov1);

    }


}

