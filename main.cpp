/*
    This file is part of the CSGO External Multihack MacOS
    Copyright (C) 2016 Gabriel Romon <mariemromon@yahoo.fr> 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License version 3
    as published by the Free Software Foundation. You may not use, modify
    or distribute this program under any other version of the
    GNU Affero General Public License.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
    You should have received a copy of the GNU Affero General Public License
    along with this program.  
*/


#include <iostream>
#include "BSPMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach_traps.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/mach.h>
#include <mach-o/dyld_images.h>
#include <libproc.h>
#include <string>
#include <ApplicationServices/ApplicationServices.h>

uint64_t Client ;
uint64_t Engine ;
uint64_t Server ;
uint64_t LocalPlayerO = 0x5149658;
uint64_t FlashDurationO = 0xABE8;
uint64_t FlagsO = 0x130;
uint64_t ForceJumpO = 0x594D980;
uint64_t ForceAttackO = 0x505A1C0;
uint64_t EntityListO = 0x50c6228;
uint64_t CrossHairIDO = 0xB370;
uint64_t HealthO = 0x12C;
uint64_t TeamNumO = 0x124;
uint64_t EnginePointerO = 0x82A720;
uint64_t IsConnectedO = 0x198;
uint64_t DormantO = 0x130;
uint64_t GlowObjectO = 0x5958f20;
uint64_t PositionO = 0x164;
uint64_t ViewAngleO = 0x8E20;
uint64_t ShotsFiredO = 0xABA0;
uint64_t AimPunchO = 0x3764;
uint64_t LifeStateO = 0x3B58; // 0x3B68
uint64_t BoneMatrixO = 0x2C70;
uint64_t ProtectedO = 0x4170;
uint64_t SpottedO = 0xEC5; //F08
uint64_t ViewOffsetO = 0x13C;
uint64_t ActiveWeaponO = 0x3628;
uint64_t WeaponIdO = 0x3788;
uint64_t ZoomLevelO = 0x3bac;

struct GlowObjectPlayer {
    uint64_t PointerToEntity;
    float red;
    float green;
    float blue;
    float alpha;
    uint8_t unk1[16];
    bool RenderWhenOccluded;
};

struct Vector {
    float x, y, z;
};

struct Matrix {
    float line1[4];
    float line2[4];
    float line3[4];
};

mach_port_t task,current;
bool space;
bool mouse1;




CGEventRef myCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{


    // The incoming keycode.
    CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    //Control
    if ( (type ==kCGEventKeyDown) && (keycode == (CGKeyCode)49)){
        space= true;
    }
    else if ( (type ==kCGEventKeyUp) && (keycode == (CGKeyCode)49)){
        space= false;
    }
    // We must return the event for it to be useful.
    CGKeyCode mouse = (CGKeyCode)CGEventGetIntegerValueField(event, kCGMouseEventPressure);
    if ((int)mouse == 255){
        mouse1 = true;
    }
    else {
        mouse1 = false;
    }

    return event;
}

void startListen(){
    CFMachPortRef	  eventTap;
    CGEventMask		eventMask;
    CFRunLoopSourceRef runLoopSource;

    // Create an event tap. We are interested in key presses.
    //(1 << kCGEventKeyDown) | (1 << kCGEventKeyUp) 
    eventMask = ( CGEventMaskBit(kCGEventLeftMouseDown) |
                 CGEventMaskBit(kCGEventLeftMouseUp));
   eventTap = CGEventTapCreate((CGEventTapLocation)kCGSessionEventTap, (CGEventTapPlacement)kCGHeadInsertEventTap, (CGEventTapOptions)0, eventMask, myCGEventCallback, NULL);
    if (!eventTap) {
        fprintf(stderr, "failed to create event tap\n");
        exit(1);
    }

    // Create a run loop source.
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    // Add to the current run loop.
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource,kCFRunLoopCommonModes);

    // Enable the event tap.
    CGEventTapEnable(eventTap, true);

    // Set it all running.
    CFRunLoopRun();
}


template <class T>
T Read(uint64_t address) {
    vm_offset_t readMem;
    T output;
    mach_msg_type_number_t size=sizeof(T);
    if (vm_read(task,address,size,&readMem,&size) == KERN_SUCCESS){
        output = (T) *(T *) readMem ;
        vm_deallocate(current,readMem,size);
        return output;
    }
    else {
        uint8_t *unk = (uint8_t*) &output;
        for (int i=0; i<sizeof(T); i++){
            *(unk + i) = -1;
        }
        return output;
        
    }

}


template <class T>
void Write(uint64_t address, T val) {
    vm_write(task,address,(vm_offset_t) &val,sizeof(T));
}

int GetCSpid() {
    int csgopid ;
    int numberOfProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    pid_t pids[numberOfProcesses];
    bzero(pids, sizeof(pids));
    proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    for (int i = 0; i < numberOfProcesses; ++i) {
        if (pids[i] == 0) { continue; }
        char pathBuffer[PROC_PIDPATHINFO_MAXSIZE];
        bzero(pathBuffer, PROC_PIDPATHINFO_MAXSIZE);
        proc_pidpath(pids[i], pathBuffer, sizeof(pathBuffer));
        char nameBuffer[256];
        int position = strlen(pathBuffer);
        while(position >= 0 && pathBuffer[position] != '/')
        {
            position--;
        }
        strcpy(nameBuffer, pathBuffer + position + 1);
        if (strcmp(nameBuffer, "csgo_osx64")==0) {
            return pids[i];
        }
    }
    
    return 0;
    
}


void GetEngineClient() {
    kern_return_t kret;
    
    mach_vm_address_t address;
    int csgopid = GetCSpid();
    task_for_pid(current_task(), getpid(), &current);
    
    kret = task_for_pid(mach_task_self(), csgopid, &task);
    if (kret!=KERN_SUCCESS)
    {
        printf("task_for_pid() failed with message %s!\n",mach_error_string(kret));
        exit(0);
    }
    
    struct task_dyld_info dyld_info;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    if (task_info(task, TASK_DYLD_INFO, (task_info_t)&dyld_info, &count) == KERN_SUCCESS)
    {
        //printf("task_info success \n");
        address = dyld_info.all_image_info_addr;
    }
    mach_msg_type_number_t size = sizeof(struct dyld_all_image_infos);
    vm_offset_t readMem;
    vm_read(task,address,size,&readMem,&size);
    struct dyld_all_image_infos* infos = (struct dyld_all_image_infos *) readMem;
    //printf ("Version: %d, %d images at offset %p\n",
    //infos->version, infos->infoArrayCount, infos->infoArray);
    size = sizeof(struct dyld_image_info) * infos->infoArrayCount;
    vm_read(task,(mach_vm_address_t) infos->infoArray,size,&readMem,&size);
    struct dyld_image_info* info = (struct dyld_image_info*) readMem;
    
    mach_msg_type_number_t sizeMax=512;
    for (int i=0; i < infos->infoArrayCount; i++) {
        vm_read(task,(mach_vm_address_t) info[i].imageFilePath,sizeMax,&readMem,&sizeMax);
        char *path = (char *) readMem ;
        //if (path)
        //    printf("path: %s %d %llx\n",path , sizeMax, (uint64_t) info[i].imageLoadAddress);
        if(strstr(path, "/client.dylib") != NULL){
            Client = (mach_vm_address_t)info[i].imageLoadAddress ;
            printf("client: 0x%llx \n", Client);
            
        }
        if(strstr(path, "/engine.dylib") != NULL){
            Engine = (mach_vm_address_t)info[i].imageLoadAddress ;
            printf("engine: 0x%llx \n", Engine);
        }
        if(strstr(path, "/server.dylib") != NULL){
            Server = (mach_vm_address_t)info[i].imageLoadAddress ;
            printf("server: 0x%llx \n", Server);
        }
        
    }
    
    
}

uint64_t EngineBase;
uint64_t LocalPlayerBase;
int IsConnected;
int MyTeamNum;
int MyHealth;
int ShotsFired;
Vector OldVectorPunch;
Vector MyPos;
Vector MyViewAngles;
int WeaponId;
int zoom;

void GetAddresses(){
    EngineBase = Read<uint64_t>(Engine + EnginePointerO);
    LocalPlayerBase = Read<uint64_t>(Client + LocalPlayerO);
    IsConnected = Read<int>(EngineBase + IsConnectedO);
    MyTeamNum = Read<int>(LocalPlayerBase + TeamNumO);
    MyHealth = Read<int>(LocalPlayerBase + HealthO);
    ShotsFired = Read<int>(LocalPlayerBase + ShotsFiredO);
    if (ShotsFired<2){
        OldVectorPunch.x = 0.0f;
        OldVectorPunch.y = 0.0f;
        OldVectorPunch.z = 0.0f;
    }
    MyPos = Read<Vector>(LocalPlayerBase + 0x164);
    MyViewAngles = Read<Vector>(EngineBase + ViewAngleO);
}


void NoFlash(){
    if (IsConnected == 6){
        float FlashDurationV = Read<float>(LocalPlayerBase + FlashDurationO);
        if (FlashDurationV > 0.0f) {
            printf("%f \n",FlashDurationV);
            Write<float>(LocalPlayerBase + FlashDurationO, 0.0f);
        }
    }
}

void Glow(){
    if (IsConnected == 6){
        uint64_t GlowPointer = Read<uint64_t>(Client + GlowObjectO);
        //printf("GlowPointer 0x%llx \n", GlowPointer);
        int ObjectCount = Read<int>(Client + GlowObjectO + 0x8);
        //printf("ObjectCount %d \n", ObjectCount);
        for (int i = 0; i < ObjectCount; i++) {
            GlowObjectPlayer GlowObj = Read<GlowObjectPlayer>(GlowPointer + i*0x40);
            uint64_t GlowObjpEntity = GlowObj.PointerToEntity;
            
            // printf("%d GlowObjpEntity %llx \n", i, GlowObjpEntity);
            int EntityTeamNum = Read<int>(GlowObjpEntity + TeamNumO) ;
            int EntityHealth = Read<int>(GlowObjpEntity + HealthO);
            if ( (EntityTeamNum >1) && (EntityTeamNum <4) && (MyTeamNum != EntityTeamNum) ) { //(MyTeamNum != EntityTeamNum)
                //printf("Local %llx %d MyTeamNum %d TeamNum %d Health %d \n", LocalPlayerBase, i, MyTeamNum, EntityTeamNum, EntityHealth);
                GlowObj.red = (100- EntityHealth)/100.0;
                GlowObj.green = EntityHealth/100.0 ;
                GlowObj.blue = 0.0f ;
                GlowObj.alpha = 1.0f ;
                GlowObj.RenderWhenOccluded = true;
                Write<GlowObjectPlayer>(GlowPointer + i*0x40, GlowObj);
            }
        }
    }
}

void Trigger(){
    if (IsConnected == 6){
        if (WeaponId == 61 || WeaponId == 32 || WeaponId == 36 || WeaponId == 4||WeaponId == 1|| WeaponId == 25 || WeaponId == 35 || WeaponId == 27 ||WeaponId == 29){
            int CrossHairID = Read<int>(LocalPlayerBase + CrossHairIDO) - 1 ;
            if ((CrossHairID > 0) && (CrossHairID < 64)) {
                uint64_t EntityPlayerBase = Read<uint64_t>(Client + EntityListO + (0x20 * CrossHairID));
                int EntityTeamNum = Read<int>(EntityPlayerBase + TeamNumO);
                //printf("%llx %d %d \n", EntityPlayerBase, EntityTeamNum, MyTeamNum);
                if ((EntityTeamNum < 4) && (MyTeamNum < 4) && (EntityTeamNum != MyTeamNum)){ /* filtre un bug sur les amis */ //(EntityTeamNum != MyTeamNum)
                    Write<int>(Client + ForceAttackO, 5);
                    usleep(200000);
                    Write<int>(Client + ForceAttackO, 4);
                    usleep(200000);
                }
            }
        }
        if ((WeaponId == 9 || WeaponId == 40)&& zoom>0){
            int CrossHairID = Read<int>(LocalPlayerBase + CrossHairIDO) - 1 ;
            if ((CrossHairID > 0) && (CrossHairID < 64)) {
                uint64_t EntityPlayerBase = Read<uint64_t>(Client + EntityListO + (0x20 * CrossHairID));
                int EntityTeamNum = Read<int>(EntityPlayerBase + TeamNumO);
                //printf("%llx %d %d \n", EntityPlayerBase, EntityTeamNum, MyTeamNum);
                if ((EntityTeamNum < 4) && (MyTeamNum < 4) && (EntityTeamNum != MyTeamNum)){ /* filtre un bug sur les amis */ //(EntityTeamNum != MyTeamNum)
                    usleep(50000);
                    Write<int>(Client + ForceAttackO, 5);
                    usleep(100000);
                    Write<int>(Client + ForceAttackO, 4);
                    
                }
            }

            
        }
    }
}

void Rapid(){
    if (IsConnected == 6){
        if (WeaponId == 2 || WeaponId == 3 || WeaponId == 4 || WeaponId == 30){
            if (mouse1){
                Write<int>(Client + ForceAttackO, 5);
                usleep(40000);
                Write<int>(Client + ForceAttackO, 4);
                //usleep(40000);
            }
        }
        
    }
}
        
void BHop(){
        if (IsConnected == 6){
            int Flag = Read<int>(LocalPlayerBase + FlagsO);
            //printf("%d \n ", Flag);
            if (space){
                //printf("%d \n ", space);
                if (Flag & (1 << 0)){
                    Write<int>(Client + ForceJumpO, 5);
                    usleep(20000);
                    Write<int>(Client + ForceJumpO, 4);
                }
            }
        }

}

Vector Normalize(Vector V){
    if (isnan(V.x)){
        V.x = 0.0f;
    }
    if (isnan(V.y)){
        V.y = 0.0f;
    }
    while (V.y < -180.0f) V.y += 360.0f;
    while (V.y > 180.0f) V.y -= 360.0f;
    if (V.x > 89.0f) V.x = 89.0f;
    if (V.x< -89.0f) V.x = -89.0f;
    V.z = 0;
    return V;
}

void RCS(){
    if (IsConnected == 6){
        if (ShotsFired >1){
            
            Vector ViewAngles;
            Vector LocalPlayerPunch = Read<Vector>(LocalPlayerBase + AimPunchO);
            ViewAngles.x = MyViewAngles.x + OldVectorPunch.x;
            ViewAngles.y = MyViewAngles.y + OldVectorPunch.y;
            
            Vector NewAngles;
            NewAngles.x = ViewAngles.x - LocalPlayerPunch.x * 2;
            NewAngles.y = ViewAngles.y - LocalPlayerPunch.y * 2;
            NewAngles.z = 0.0f;
            
            Vector NewAnglesN = Normalize(NewAngles);
            Write<Vector>(EngineBase + ViewAngleO , NewAnglesN);

            OldVectorPunch.x = LocalPlayerPunch.x*2;
            OldVectorPunch.y = LocalPlayerPunch.y*2;
        }
    }
}

float Convert( float angle){
    return angle * (180.0 / M_PI);
}

float Norm(Vector V){
    return sqrtf(V.x * V.x + V.y * V.y + V.z * V.z);
}

Vector Sub(Vector V1, Vector V2){
    Vector V3;
    V3.x = V1.x - V2.x;
    V3.y = V1.y - V2.y;
    V3.z = V1.z - V2.z;
    return V3;
}

void AimEntity(int i){
    uint64_t EntityPlayerBase = Read<uint64_t>(Client + EntityListO + (0x20 * i));
    int EntityState = Read<int>(EntityPlayerBase + LifeStateO);
    if  (EntityState==0){
        EntityState = Read<int>(EntityPlayerBase + LifeStateO);
        uint64_t BoneMatrix = Read<uint64_t>(EntityPlayerBase + BoneMatrixO);
        uint64_t HeadBone = BoneMatrix + 6*0x30;
        
        Matrix BotPos = Read<Matrix>(HeadBone);
        
        
        Vector AimVector;
        AimVector.x = BotPos.line1[3] - MyPos.x;
        AimVector.y = BotPos.line2[3] - MyPos.y;
        AimVector.z = BotPos.line3[3] - MyPos.z - Read<float>(LocalPlayerBase + ViewOffsetO);
        float norm = Norm(AimVector);
        float View1 = asin(-AimVector.z/norm);
        float View2;
        if (AimVector.y>0){
            View2 = acos(AimVector.x/(norm * cos(View1)));
        }
        else{
            View2 = -acos(AimVector.x/(norm * cos(View1)));
        }
        
        Vector ViewAngles;
        ViewAngles.x = Convert(View1);
        ViewAngles.y = Convert(View2);
        ViewAngles.z = 0;
        Write<Vector>(EngineBase + ViewAngleO, Normalize(ViewAngles));
    }

}

int NPlayers;
struct Lock{
    float dist;
    int id;
};


int LookUpPlayers(){
    Lock Candidate;
    Candidate.dist = FLT_MAX;
    Candidate.id = -1;
    int res = 0;
    for(int i=0; i<66; i++){
        uint64_t EntityPlayerBase = Read<uint64_t>(Client + EntityListO + (0x20 * (i+1)));
        if (EntityPlayerBase != 0){
            int EntityState = Read<int>(EntityPlayerBase + LifeStateO);
            int EntityTeamNum = Read<int>(EntityPlayerBase + TeamNumO);
            if ((EntityTeamNum != -1) && ((EntityTeamNum == 2) || (EntityTeamNum == 3)) && (EntityTeamNum != MyTeamNum) && (EntityState==0)){
                res = res + 1;
                
                uint64_t BoneMatrix = Read<uint64_t>(EntityPlayerBase + BoneMatrixO);
                uint64_t HeadBone = BoneMatrix + 6*0x30;
                Matrix BotHeadPos = Read<Matrix>(HeadBone);
                Vector BotPos;
                BotPos.x = BotHeadPos.line1[3];
                BotPos.y = BotHeadPos.line2[3];
                BotPos.z = BotHeadPos.line3[3] - Read<float>(LocalPlayerBase + ViewOffsetO);
                                
                Vector AimVector = Sub(BotPos, MyPos);
                float norm = Norm(AimVector);
                float View1 = asin(-AimVector.z/norm);
                float View2;
                if (AimVector.y>0){
                    View2 = acos(AimVector.x/(norm * cos(View1)));
                }
                else{
                    View2 = -acos(AimVector.x/(norm * cos(View1)));
                }
                
                Vector PotentialViewAngles;
                PotentialViewAngles.x = Convert(View1);
                PotentialViewAngles.y = Convert(View2);
                
                float MyVector[3] = {MyPos.x,MyPos.y,MyPos.z + Read<float>(LocalPlayerBase + ViewOffsetO)};
                float PVector[3] = {BotPos.x,BotPos.y,BotPos.z};
                float * MyP = MyVector;
                float * PP = PVector;
//                bool test = g_pBSP->Visible(MyP, PP);
                
                if ((fabsf(PotentialViewAngles.y - MyViewAngles.y)<1.5) && Candidate.dist>norm ){
                    Candidate.dist = norm;
                    Candidate.id = i+1;
                }

            }
            
        }
    }
    NPlayers = res;
    return Candidate.id;
    
    
}

void Aim(){
    int id = LookUpPlayers();
    if (id>0){
        AimEntity(id);
    }
}

void GetWeaponId(){
    if (IsConnected == 6){
        uint64_t weaponHandle = Read<uint64_t>(LocalPlayerBase + ActiveWeaponO);
        int weaponEntID = weaponHandle & 0xFFF;
        uint64_t weaponEnt = Read<uint64_t>(Client + EntityListO + (weaponEntID - 1) * 0x20);
        WeaponId = Read<int>(weaponEnt + WeaponIdO);
        zoom = Read<int>(weaponEnt + ZoomLevelO);
    }
}

int main(int argc, const char * argv[]) {
    GetEngineClient();
    g_pBSP->load("lol","lol");
    g_pBSP->DisplayInfo();
    dispatch_queue_t MainQueue = dispatch_queue_create("MainQueue",NULL);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        startListen();
    });
    while(1){
        dispatch_async(MainQueue,^{
                    GetAddresses();
                });
        dispatch_async(MainQueue,^{
           GetWeaponId();
        });

        dispatch_async(MainQueue,^{

            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                Glow();
            });
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                NoFlash();
            });
//            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
//                BHop();
//            });
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                RCS();
            });
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                Trigger();
            });
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                Rapid();
            });
//            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
//                Aim();
//            });
        });
        usleep(15000);

    }
}
