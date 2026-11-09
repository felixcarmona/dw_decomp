/* Included by model.c. Only draw matrices are temporarily changed: source SRT,
 * momentum, navigation, events, animation timing and texture moves stay native. */
#include "smooth_math.h"
extern int32_t MOD_FPS_TICKS, MOD_RENDER_ALPHA, IS_IN_MENU;
extern int8_t GAME_STATE;

#define SMOOTH_BONES 63
typedef struct {
    Entity *entity;
    PositionData *positions;
    void *animation;
    int32_t type, map, start, count, floor, lastFloor, lastTick;
    uint8_t fromValid[SMOOTH_BONES], lastValid[SMOOTH_BONES];
    SmoothPose pose[0];           /* source then last displayed, count-1 each */
} SmoothState;
static SmoothState *smoothStates[ENTITY_MAX];
static MATRIX smoothSaved[SMOOTH_BONES+1]; /* draw calls are sequential */
int32_t MOD_SMOOTH_TRANSITIONS = 1;
/* model.c has no vanilla small-BSS region in the decomp linker script. */
int32_t MOD_SMOOTH_STARTS __attribute__((section(".bss.modSmoothStats")));
int32_t MOD_SMOOTH_DRAWS __attribute__((section(".bss.modSmoothStats")));
int32_t MOD_SMOOTH_FLOOR_ADJUSTS __attribute__((section(".bss.modSmoothStats")));

static int32_t smoothSlot(int32_t s)
{
    return s==0 || s==2 || s==3 || s==4 || s==5 || s==18;
}

void modSmoothForget(int32_t instance)
{
    if (instance>=0 && instance<ENTITY_MAX && smoothStates[instance]) {
        free3(smoothStates[instance]); smoothStates[instance]=NULL;
    }
}

void modSmoothMapReset(void)
{
    int32_t i;
    for (i=0;i<ENTITY_MAX;i++) modSmoothForget(i);
}

static int32_t smoothEligible(Entity *e)
{
    /* All imported species, including future imports; vanilla/tamer untouched. */
    return MOD_SMOOTH_TRANSITIONS && e && e->type>=180 && e->posData &&
        GAME_STATE==0 && !IS_IN_MENU && e->flatSprite==-1 &&
        DIGI_PARA(e->type).boneCount>=2 &&
        DIGI_PARA(e->type).boneCount<=SMOOTH_BONES+1;
}

static int32_t smoothIdentity(SmoothState *s, Entity *e)
{
    return s && s->entity==e && s->positions==e->posData &&
        s->type==e->type && s->animation==e->animPtr &&
        s->map==MOD_INTERP_MAP_GEN && s->count==DIGI_PARA(e->type).boneCount;
}

/* Actual transformed mesh, rather than bone origins or an anatomy guess.
 * Y points down in DW1. Return its lowest visible vertex relative to root Y.
 * Called only during the short transition, never during steady locomotion. */
static int32_t smoothFloor(Entity *e, int32_t n)
{
    int32_t b,j,y,bottom=-0x3fffffff;
    PositionData *pd=e->posData;
    MATRIX lw,ls;
    struct TMD_STRUCT *obj;
    SVECTOR *v;
    for(b=0;b<n;b++) {
        obj=pd[b].obj.tmd;
        if (!obj || (pd[b].obj.attribute & GsDOFF)) continue;
        GsGetLws(pd[b].obj.coord2,&lw,&ls);
        /* Collapsed parts have no visible area and must not become support. */
        if ((!lw.m[0][0] && !lw.m[1][0] && !lw.m[2][0]) ||
            (!lw.m[0][1] && !lw.m[1][1] && !lw.m[2][1]) ||
            (!lw.m[0][2] && !lw.m[1][2] && !lw.m[2][2])) continue;
        v=(SVECTOR *)obj->vertop;
        for(j=0;j<(int32_t)obj->vern;j++,v++) {
            y=(((int32_t)lw.m[1][0]*v->vx+(int32_t)lw.m[1][1]*v->vy+
                (int32_t)lw.m[1][2]*v->vz)>>12)+lw.t[1];
            if(y>bottom) bottom=y;
        }
    }
    return bottom==-0x3fffffff ? 0 : bottom-pd[0].posMatrix.coord.t[1];
}

void modSmoothBeforeStart(void *entity, int32_t destination)
{
    Entity *e=entity;
    SmoothState *s;
    PositionData *pd;
    int32_t i,b,n,continuing;
    uint8_t valid[SMOOTH_BONES]={0};
    for(i=0;i<ENTITY_MAX;i++) if(ENTITY_TABLE[i]==e) break;
    if(i==ENTITY_MAX) return;
    s=smoothStates[i];
    if(!smoothEligible(e) || !smoothSlot(destination) || !smoothSlot(e->anim.animId)) {
        modSmoothForget(i); return;
    }
    /* Repeating a start in its first frame is not another motion transition. */
    if(destination==e->anim.animId && e->anim.animFrame<=1) return;
    n=DIGI_PARA(e->type).boneCount;
    continuing=smoothIdentity(s,e) && MOD_FPS_TICKS-s->lastTick<=1 &&
        MOD_FPS_TICKS>=s->lastTick;
    if(!continuing) {
        modSmoothForget(i);
        s=malloc3(sizeof(*s)+2*(n-1)*sizeof(SmoothPose));
        if(!s) return; /* A tight map keeps its original renderer. */
        smoothStates[i]=s;
    }
    pd=e->posData;
    if(continuing) {
        memcpy(s->pose,s->pose+n-1,(n-1)*sizeof(SmoothPose));
        memcpy(valid,s->lastValid,sizeof(valid)); s->floor=s->lastFloor;
    } else {
        for(b=1;b<n;b++) {
            int32_t signs[3];
            signs[0]=pd[b].scale.vx; signs[1]=pd[b].scale.vy; signs[2]=pd[b].scale.vz;
            if(smoothRead(pd[b].posMatrix.coord.m,pd[b].posMatrix.coord.t,signs,&s->pose[b-1]))
                valid[b-1]=1;
        }
        modFlagBones(pd,n); s->floor=smoothFloor(e,n);
    }
    s->entity=e; s->positions=pd; s->animation=e->animPtr;
    s->type=e->type; s->map=MOD_INTERP_MAP_GEN; s->count=n;
    s->start=MOD_FPS_TICKS+1; memcpy(s->fromValid,valid,sizeof(valid));
    /* A second start in this same tick must still use the captured pose. */
    memcpy(s->pose+n-1,s->pose,(n-1)*sizeof(SmoothPose));
    s->lastFloor=s->floor; memcpy(s->lastValid,valid,sizeof(valid)); s->lastTick=MOD_FPS_TICKS;
    MOD_SMOOTH_STARTS++;
}

static int32_t modSmoothDrawBegin(int32_t instance)
{
    Entity *e=ENTITY_TABLE[instance];
    SmoothState *s=smoothStates[instance];
    PositionData *pd;
    int32_t b,n,w,elapsed,targetFloor,actualFloor,expectedFloor;
    SmoothPose target,result;
    if(!s) return 0;
    if(!smoothEligible(e) || !smoothIdentity(s,e) || !smoothSlot(e->anim.animId)) {
        modSmoothForget(instance); return 0;
    }
    elapsed=(MOD_FPS_TICKS-s->start)*4096+MOD_RENDER_ALPHA;
    if(elapsed<0 || elapsed>=4*4096) { modSmoothForget(instance); return 0; }
    w=smoothWeight(elapsed); n=s->count; pd=e->posData;
    modFlagBones(pd,n); targetFloor=smoothFloor(e,n);
    memset(s->lastValid,0,sizeof(s->lastValid));
    for(b=0;b<n;b++) smoothSaved[b]=pd[b].posMatrix.coord;
    for(b=1;b<n;b++) {
        int32_t signs[3];
        signs[0]=pd[b].scale.vx; signs[1]=pd[b].scale.vy; signs[2]=pd[b].scale.vz;
        if(!smoothRead(smoothSaved[b].m,smoothSaved[b].t,signs,&target)) continue;
        if(s->fromValid[b-1]) {
            smoothBlend(&s->pose[b-1],&target,w,&result);
            smoothMatrix(&result,pd[b].posMatrix.coord.m,pd[b].posMatrix.coord.t);
        } else result=target;
        s->pose[n+b-2]=result; s->lastValid[b-1]=1;
    }
    modFlagBones(pd,n); actualFloor=smoothFloor(e,n);
    expectedFloor=smoothLerp(s->floor,targetFloor,w);
    /* Preserve the interpolated native support/flight height. Does not claim IK
     * foot planting: contacts and terrain are still owned by the native model. */
    pd[0].posMatrix.coord.t[1]+=expectedFloor-actualFloor;
    if(expectedFloor!=actualFloor) MOD_SMOOTH_FLOOR_ADJUSTS++;
    s->lastFloor=expectedFloor; s->lastTick=MOD_FPS_TICKS;
    modFlagBones(pd,n); MOD_SMOOTH_DRAWS++;
    return n;
}

static void modSmoothDrawEnd(int32_t instance, int32_t count)
{
    int32_t b;
    PositionData *pd=ENTITY_TABLE[instance]->posData;
    if(!count) return;
    for(b=0;b<count;b++) pd[b].posMatrix.coord=smoothSaved[b];
    modFlagBones(pd,count);
}
