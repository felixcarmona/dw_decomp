/* Render-only pose transitions. Q12 quaternion nlerp avoids Euler wrap flips.
 * Portable integer code shared by the PS1 build and the offline geometry audit. */
#ifndef MOD_SMOOTH_MATH_H
#define MOD_SMOOTH_MATH_H
#include <stdint.h>

typedef struct {
    int16_t q[4];                 /* x,y,z,w */
    int16_t s[3];
    int32_t t[3];
} SmoothPose;

static uint32_t smoothSqrt(uint32_t n)
{
    uint32_t r = 0, bit = 1u << 30;
    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) { n -= r + bit; r = (r >> 1) + bit; }
        else r >>= 1;
        bit >>= 2;
    }
    return r;
}

static int32_t smoothWeight(int32_t elapsed)
{
    int32_t x;
    if (elapsed <= 0) return 0;
    if (elapsed >= 4 * 4096) return 4096;
    x = elapsed / 4;
    return (int32_t)(((int64_t)x*x*(12288-2*x) + 8388608) >> 24);
}

static int32_t smoothLerp(int32_t a, int32_t b, int32_t w)
{
    /* Translations are 32 bit; never narrow them to the old FPS cache's short. */
    return a + (int32_t)(((int64_t)(b-a)*w + 2048) >> 12);
}

static void smoothQuat(const int16_t m[3][3], int16_t q[4])
{
    int32_t i, j, k, s, v[4], trace = m[0][0]+m[1][1]+m[2][2];
    if (trace > 0) {
        s = smoothSqrt((4096+trace)*4096);
        v[3] = s/2;
        v[0] = (m[2][1]-m[1][2])*2048/s;
        v[1] = (m[0][2]-m[2][0])*2048/s;
        v[2] = (m[1][0]-m[0][1])*2048/s;
    } else {
        i = m[1][1] > m[0][0] ? 1 : 0;
        if (m[2][2] > m[i][i]) i = 2;
        j = (i+1)%3; k = (j+1)%3;
        s = smoothSqrt((4096+m[i][i]-m[j][j]-m[k][k])*4096);
        if (!s) { q[0]=q[1]=q[2]=0; q[3]=4096; return; }
        v[i] = s/2;
        v[j] = (m[j][i]+m[i][j])*2048/s;
        v[k] = (m[k][i]+m[i][k])*2048/s;
        v[3] = (m[k][j]-m[j][k])*2048/s;
    }
    s = smoothSqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]);
    for (i=0;i<4;i++) q[i] = v[i]*4096/s;
}

/* Decode an existing local render matrix, including ordinary FPS interpolation.
 * Mirrored/hidden/degenerate parts use their destination unchanged. */
static int32_t smoothRead(const int16_t m[3][3], const int32_t t[3],
                          const int32_t scaleSign[3], SmoothPose *p)
{
    int32_t i,j,s; int16_t r[3][3];
    for (j=0;j<3;j++) {
        if (scaleSign[j] <= 0) return 0;
        s = smoothSqrt((int32_t)m[0][j]*m[0][j] +
                       (int32_t)m[1][j]*m[1][j] + (int32_t)m[2][j]*m[2][j]);
        if (s < 4 || s > 32767) return 0;
        p->s[j] = s; p->t[j] = t[j];
        for (i=0;i<3;i++) r[i][j] = (int32_t)m[i][j]*4096/s;
    }
    smoothQuat(r,p->q);
    return 1;
}

static void smoothBlend(const SmoothPose *a, const SmoothPose *b,
                         int32_t w, SmoothPose *p)
{
    int32_t i, dot=0, q[4], len=0, sign;
    if (w<=0) { *p=*a; return; }
    if (w>=4096) { *p=*b; return; }
    for (i=0;i<4;i++) dot += (int32_t)a->q[i]*b->q[i];
    sign = dot<0 ? -1:1;
    for (i=0;i<4;i++) {
        q[i]=smoothLerp(a->q[i],sign*b->q[i],w); len+=q[i]*q[i];
    }
    len=smoothSqrt(len);
    for (i=0;i<4;i++) p->q[i]=q[i]*4096/len;
    for (i=0;i<3;i++) {
        p->t[i]=smoothLerp(a->t[i],b->t[i],w);
        p->s[i]=smoothLerp(a->s[i],b->s[i],w);
    }
}

static void smoothMatrix(const SmoothPose *p, int16_t m[3][3], int32_t t[3])
{
    int32_t i,j,x=p->q[0],y=p->q[1],z=p->q[2],w=p->q[3],r[3][3];
    r[0][0]=4096-((2*(y*y+z*z))>>12);
    r[0][1]=(2*(x*y-z*w))>>12; r[0][2]=(2*(x*z+y*w))>>12;
    r[1][0]=(2*(x*y+z*w))>>12; r[1][1]=4096-((2*(x*x+z*z))>>12);
    r[1][2]=(2*(y*z-x*w))>>12;
    r[2][0]=(2*(x*z-y*w))>>12; r[2][1]=(2*(y*z+x*w))>>12;
    r[2][2]=4096-((2*(x*x+y*y))>>12);
    for (j=0;j<3;j++) {
        t[j]=p->t[j];
        for(i=0;i<3;i++) m[i][j]=(r[i][j]*p->s[j]+2048)>>12;
    }
}
#endif
